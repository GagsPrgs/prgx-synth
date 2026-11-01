#include "PluginProcessor.h"
#include "ParameterIds.h"
#include "PluginEditor.h"
#include "SynthSound.h"
#include "SynthVoice.h"

PrgxAudioProcessor::PrgxAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Pokazivač na vrijednost parametra uzimamo jednom, ovdje, a ne na audio niti.
    gainParameter = parameters.getRawParameterValue (ParameterIds::gain);

    // Glasove i zvuk dodajemo ovdje, prije nego što audio nit krene (ne smiju se dodavati kasnije).
    for (int i = 0; i < numVoices; ++i)
        synth.addVoice (new SynthVoice (parameters));

    synth.addSound (new SynthSound());
}

namespace
{
    // Postotak se prikazuje kao "70 %".
    juce::String percentToText (float value, int)
    {
        return juce::String (juce::roundToInt (value * 100.0f)) + " %";
    }

    float textToPercent (const juce::String& text)
    {
        return text.getFloatValue() / 100.0f;
    }

    // Vrijeme se prikazuje kao "12 ms" ispod sekunde, a inače kao "1.50 s".
    juce::String secondsToText (float seconds, int)
    {
        if (seconds < 1.0f)
            return juce::String (juce::roundToInt (seconds * 1000.0f)) + " ms";

        return juce::String (seconds, 2) + " s";
    }

    // Upisani tekst bez oznake "ms" shvaća se kao sekunde.
    float textToSeconds (const juce::String& text)
    {
        const float number = text.getFloatValue();
        return text.containsIgnoreCase ("ms") ? number / 1000.0f : number;
    }

    // Frekvencija se prikazuje kao "800 Hz" ili "1.50 kHz".
    juce::String hzToText (float hz, int)
    {
        if (hz < 1000.0f)
            return juce::String (juce::roundToInt (hz)) + " Hz";

        return juce::String (hz / 1000.0f, 2) + " kHz";
    }

    // Upisani tekst s oznakom "k" (npr. "1.5k" ili "1.5 kHz") shvaća se kao kiloherci, inače kao herci.
    float textToHz (const juce::String& text)
    {
        const float number = text.getFloatValue();
        return text.containsIgnoreCase ("k") ? number * 1000.0f : number;
    }

    // Cutoff ide od 20 Hz do 20 kHz. Gumb je "nagnut" tako da mu je sredina 1 kHz.
    // Zadano je posve otvoren (20 kHz), pa filtar zasad ne mijenja zvuk.
    std::unique_ptr<juce::AudioParameterFloat> makeCutoffParameter (const char* id, const juce::String& name)
    {
        juce::NormalisableRange<float> range (20.0f, 20000.0f);
        range.setSkewForCentre (1000.0f);

        return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name, range, 20000.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (hzToText)
                                                 .withValueFromStringFunction (textToHz));
    }

    std::unique_ptr<juce::AudioParameterFloat> makePercentParameter (const char* id, const juce::String& name, float defaultValue)
    {
        return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name,
            juce::NormalisableRange<float> (0.0f, 1.0f), defaultValue,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (percentToText)
                                                 .withValueFromStringFunction (textToPercent));
    }

    // Vremena idu od 1 ms do maxSeconds. Gumb je "nagnut" tako da mu je sredina baš centreSeconds,
    // pa ima dovoljno preciznosti i za kratka i za duga vremena.
    std::unique_ptr<juce::AudioParameterFloat> makeTimeParameter (const char* id, const juce::String& name,
                                                                  float maxSeconds, float centreSeconds, float defaultSeconds)
    {
        juce::NormalisableRange<float> range (0.001f, maxSeconds);
        range.setSkewForCentre (centreSeconds);

        return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name, range, defaultSeconds,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (secondsToText)
                                                 .withValueFromStringFunction (textToSeconds));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout PrgxAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Redoslijed valnih oblika mora odgovarati enumu Waveform u SynthVoice.cpp. Zadano je pila (indeks 1).
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParameterIds::waveform, 1 }, "Waveform",
        juce::StringArray { "Sine", "Saw", "Square", "Triangle" }, 1));

    // Za koliko poluglasa kotačić pomiče ton. FL Studio šalje pitch bend prema rasponu svog kanala,
    // pa ovdje treba biti ista vrijednost. Redoslijed mora odgovarati popisu u SynthVoice.cpp. Zadano je 2 (indeks 1).
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParameterIds::bendRange, 1 }, "Bend Range",
        juce::StringArray { "1 st", "2 st", "3 st", "4 st", "5 st", "7 st", "12 st", "24 st" }, 1));

    layout.add (makeCutoffParameter (ParameterIds::cutoff, "Cutoff"));
    layout.add (makePercentParameter (ParameterIds::resonance, "Resonance", 0.0f));

    layout.add (makeTimeParameter (ParameterIds::attack, "Attack", 5.0f, 0.15f, 0.01f));
    layout.add (makeTimeParameter (ParameterIds::decay, "Decay", 5.0f, 0.3f, 0.3f));
    layout.add (makePercentParameter (ParameterIds::sustain, "Sustain", 0.7f));
    layout.add (makeTimeParameter (ParameterIds::release, "Release", 10.0f, 0.5f, 0.25f));
    layout.add (makePercentParameter (ParameterIds::gain, "Volume", 0.8f));

    return layout;
}

void PrgxAudioProcessor::prepareToPlay (double sampleRate, int)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    previousGain = gainParameter->load();
}

void PrgxAudioProcessor::releaseResources()
{
}

bool PrgxAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void PrgxAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();

    // Synthesiser čita note iz MIDI poruka i zbraja zvuk svih glasova u buffer.
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // Glasnoća se mijenja postupno kroz blok kako ne bi "pucketalo" kad se okrene gumb.
    const float targetGain = gainParameter->load();
    buffer.applyGainRamp (0, buffer.getNumSamples(), previousGain, targetGain);
    previousGain = targetGain;
}

juce::AudioProcessorEditor* PrgxAudioProcessor::createEditor()
{
    return new PrgxAudioProcessorEditor (*this);
}

void PrgxAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void PrgxAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

// JUCE poziva ovu funkciju kad FL Studio učita plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PrgxAudioProcessor();
}
