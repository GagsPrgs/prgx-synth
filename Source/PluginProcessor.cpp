#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SynthSound.h"
#include "SynthVoice.h"

PrgxAudioProcessor::PrgxAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Pokazivač na vrijednost parametra uzimamo jednom, ovdje, a ne na audio niti.
    gainParameter = parameters.getRawParameterValue ("gain");

    // Glasove i zvuk dodajemo ovdje, prije nego što audio nit krene (ne smiju se dodavati kasnije).
    for (int i = 0; i < numVoices; ++i)
        synth.addVoice (new SynthVoice());

    synth.addSound (new SynthSound());
}

juce::AudioProcessorValueTreeState::ParameterLayout PrgxAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "gain", 1 }, "Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f), 0.8f));

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
