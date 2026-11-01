#include "SynthVoice.h"
#include "ParameterIds.h"
#include "SynthSound.h"

namespace
{
    // Redoslijed mora odgovarati popisu valnih oblika u PluginProcessor.cpp (createParameterLayout).
    enum Waveform { sine, saw, square, triangle };

    // Običan pilasti ili kvadratni val ima oštre skokove, a oni na višim notama
    // stvaraju neželjene visoke tonove (aliasing). PolyBLEP ublaži skok baš oko tog mjesta.
    // t je faza (od 0 do 1), a dt pomak faze po uzorku.
    double polyBlep (double t, double dt)
    {
        if (t < dt)
        {
            t /= dt;
            return t + t - t * t - 1.0;
        }

        if (t > 1.0 - dt)
        {
            t = (t - 1.0) / dt;
            return t * t + t + t + 1.0;
        }

        return 0.0;
    }

    // Jedan uzorak oscilatora za zadanu fazu. Valni oblici su približno izjednačeni po glasnoći.
    double oscillator (Waveform waveform, double phase, double phaseIncrement)
    {
        switch (waveform)
        {
            case sine:
                return 0.8 * std::sin (juce::MathConstants<double>::twoPi * phase);

            case square:
            {
                const double edges = polyBlep (phase, phaseIncrement)
                                     - polyBlep (std::fmod (phase + 0.5, 1.0), phaseIncrement);
                return 0.6 * ((phase < 0.5 ? 1.0 : -1.0) + edges);
            }

            case triangle:
                return 4.0 * std::abs (std::fmod (phase + 0.75, 1.0) - 0.5) - 1.0;

            case saw:
            default:
                // Raste od -1 do +1, pa skoči natrag.
                return (2.0 * phase - 1.0) - polyBlep (phase, phaseIncrement);
        }
    }

    Waveform readWaveform (const std::atomic<float>& parameter)
    {
        return static_cast<Waveform> (juce::jlimit (0, (int) triangle, juce::roundToInt (parameter.load())));
    }

    // Ponuđeni rasponi pitch benda u poluglasima. Redoslijed mora odgovarati popisu u PluginProcessor.cpp.
    constexpr double bendRanges[] { 1.0, 2.0, 3.0, 4.0, 5.0, 7.0, 12.0, 24.0 };

    double readBendRange (const std::atomic<float>& parameter)
    {
        return bendRanges[juce::jlimit (0, (int) std::size (bendRanges) - 1, juce::roundToInt (parameter.load()))];
    }

    // Položaj kotačića (od 0 do 16383, sredina je 8192) pretvara u raspon od -1 do +1.
    double pitchWheelToBend (int wheelValue)
    {
        return (wheelValue - 8192) / 8192.0;
    }
}

SynthVoice::SynthVoice (juce::AudioProcessorValueTreeState& state)
    : waveformParameter (state.getRawParameterValue (ParameterIds::waveform)),
      bendRangeParameter (state.getRawParameterValue (ParameterIds::bendRange)),
      cutoffParameter (state.getRawParameterValue (ParameterIds::cutoff)),
      resonanceParameter (state.getRawParameterValue (ParameterIds::resonance)),
      attackParameter (state.getRawParameterValue (ParameterIds::attack)),
      decayParameter (state.getRawParameterValue (ParameterIds::decay)),
      sustainParameter (state.getRawParameterValue (ParameterIds::sustain)),
      releaseParameter (state.getRawParameterValue (ParameterIds::release))
{
}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*> (sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    const double bend = pitchWheelToBend (currentPitchWheelPosition);
    bendSemitones = readBendRange (*bendRangeParameter);

    phase = 0.0;
    baseIncrement = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber) / getSampleRate();
    phaseIncrement = baseIncrement * std::exp2 (bend * bendSemitones / 12.0);
    level = velocity;

    // Nova nota kreće odmah s pravim kotačićem i cutoffom, a ne "klizi" od prijašnje note.
    smoothedPitchBend.setCurrentAndTargetValue (bend);
    smoothedCutoff.setCurrentAndTargetValue (cutoffParameter->load());
    smoothedFilterMix.setCurrentAndTargetValue (LowPassFilter::isOpen (cutoffParameter->load(), resonanceParameter->load()) ? 0.0f : 1.0f);

    filter.reset();
    appliedResonance = -1.0f;

    updateEnvelope();
    envelope.noteOn();
}

void SynthVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        // Nota se utišava, a glas se oslobodi u renderNextBlock kad ovojnica završi.
        updateEnvelope();
        envelope.noteOff();
    }
    else
    {
        // Glas mora odmah zašutjeti (npr. kad ga synth preuzme za novu notu).
        envelope.reset();
        clearCurrentNote();
    }
}

void SynthVoice::pitchWheelMoved (int newPitchWheelValue)
{
    smoothedPitchBend.setTargetValue (pitchWheelToBend (newPitchWheelValue));
}

void SynthVoice::setCurrentPlaybackSampleRate (double newRate)
{
    SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);

    // Synthesiser nas pozove i s 0 dok brzina uzorkovanja još nije poznata.
    if (newRate > 0.0)
    {
        envelope.setSampleRate (newRate);

        // Nova brzina uzorkovanja traži preračun, pa ovdje ne uspoređujemo s prijašnjim vrijednostima.
        envelope.setParameters (readEnvelopeParameters());

        smoothedPitchBend.reset (newRate, 0.005);
        smoothedCutoff.reset (newRate, 0.01);
        smoothedFilterMix.reset (newRate, 0.01);
    }
}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    // Synthesiser pozove sve glasove, pa oni koji ne sviraju moraju odmah odustati.
    if (! isVoiceActive())
        return;

    updateEnvelope();

    const auto waveform = readWaveform (*waveformParameter);
    const float gain = level * voiceGain;
    const double sampleRate = getSampleRate();

    const float cutoffHz = cutoffParameter->load();
    const float resonance = resonanceParameter->load();

    smoothedCutoff.setTargetValue (cutoffHz);
    smoothedFilterMix.setTargetValue (LowPassFilter::isOpen (cutoffHz, resonance) ? 0.0f : 1.0f);

    bool filterNeedsUpdate = ! juce::exactlyEqual (resonance, appliedResonance);
    appliedResonance = resonance;

    // Raspon kotačića se može promijeniti i dok nota svira, pa visinu tona preračunamo na početku svakog bloka.
    bendSemitones = readBendRange (*bendRangeParameter);
    phaseIncrement = baseIncrement * std::exp2 (smoothedPitchBend.getCurrentValue() * bendSemitones / 12.0);

    for (int i = 0; i < numSamples; ++i)
    {
        if (smoothedPitchBend.isSmoothing())
            phaseIncrement = baseIncrement * std::exp2 (smoothedPitchBend.getNextValue() * bendSemitones / 12.0);

        // Koeficijente filtra računamo samo dok se nešto mijenja, jer je to najskuplji dio filtra.
        if (filterNeedsUpdate || smoothedCutoff.isSmoothing())
        {
            filter.setParameters (smoothedCutoff.getNextValue(), resonance, sampleRate);
            filterNeedsUpdate = false;
        }

        // Filtar radi stalno (da mu je stanje spremno), a mix određuje koliko ga čujemo.
        const float raw = static_cast<float> (oscillator (waveform, phase, phaseIncrement));
        const float filtered = filter.process (raw);
        const float sample = (raw + smoothedFilterMix.getNextValue() * (filtered - raw)) * envelope.getNextSample() * gain;

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            outputBuffer.addSample (channel, startSample + i, sample);

        phase += phaseIncrement;

        if (phase >= 1.0)
            phase -= 1.0;

        // Ovojnica je gotova: glas je slobodan za sljedeću notu.
        if (! envelope.isActive())
        {
            clearCurrentNote();
            break;
        }
    }
}

juce::ADSR::Parameters SynthVoice::readEnvelopeParameters() const
{
    return { attackParameter->load(), decayParameter->load(), sustainParameter->load(), releaseParameter->load() };
}

void SynthVoice::updateEnvelope()
{
    const auto wanted = readEnvelopeParameters();
    const auto& current = envelope.getParameters();

    // setParameters preračuna brzine ovojnice, pa bi usred otpuštanja promijenio njegov nagib.
    // Zato ga zovemo samo kad se neki gumb stvarno pomaknuo.
    if (! (juce::exactlyEqual (wanted.attack, current.attack)
           && juce::exactlyEqual (wanted.decay, current.decay)
           && juce::exactlyEqual (wanted.sustain, current.sustain)
           && juce::exactlyEqual (wanted.release, current.release)))
    {
        envelope.setParameters (wanted);
    }
}
