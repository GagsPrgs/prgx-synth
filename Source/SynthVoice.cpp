#include "SynthVoice.h"
#include "ParameterIds.h"
#include "SynthSound.h"

namespace
{
    // Običan pilasti val ima oštar skok na kraju svakog titraja, a on na višim notama
    // stvara neželjene visoke tonove (aliasing). PolyBLEP ublaži skok baš oko tog mjesta.
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
}

SynthVoice::SynthVoice (juce::AudioProcessorValueTreeState& state)
    : attackParameter (state.getRawParameterValue (ParameterIds::attack)),
      decayParameter (state.getRawParameterValue (ParameterIds::decay)),
      sustainParameter (state.getRawParameterValue (ParameterIds::sustain)),
      releaseParameter (state.getRawParameterValue (ParameterIds::release))
{
}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*> (sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    phase = 0.0;
    phaseIncrement = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber) / getSampleRate();
    level = velocity;

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

void SynthVoice::setCurrentPlaybackSampleRate (double newRate)
{
    SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);

    // Synthesiser nas pozove i s 0 dok brzina uzorkovanja još nije poznata.
    if (newRate > 0.0)
    {
        envelope.setSampleRate (newRate);

        // Nova brzina uzorkovanja traži preračun, pa ovdje ne uspoređujemo s prijašnjim vrijednostima.
        envelope.setParameters (readEnvelopeParameters());
    }
}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    // Synthesiser pozove sve glasove, pa oni koji ne sviraju moraju odmah odustati.
    if (! isVoiceActive())
        return;

    updateEnvelope();

    const float gain = level * voiceGain;

    for (int i = 0; i < numSamples; ++i)
    {
        // Pilasti val: raste od -1 do +1, pa skoči natrag (uz PolyBLEP ispravak oko skoka).
        const double saw = (2.0 * phase - 1.0) - polyBlep (phase, phaseIncrement);
        const float sample = static_cast<float> (saw) * envelope.getNextSample() * gain;

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
