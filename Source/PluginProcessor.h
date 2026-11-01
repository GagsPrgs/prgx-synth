#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

// "Mozak" plugina: ovdje se obrađuje zvuk i čuvaju parametri.
class PrgxAudioProcessor : public juce::AudioProcessor
{
public:
    PrgxAudioProcessor();
    ~PrgxAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    // Poziva se na audio niti, stotine puta u sekundi. Vidi pravila u CLAUDE.md.
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    // Spremanje/učitavanje postavki u FL Studio projekt (.flp).
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Svi parametri plugina (zasad samo glasnoća).
    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    static constexpr int numVoices = 16;  // koliko nota može zvučati istovremeno

    juce::Synthesiser synth;

    std::atomic<float>* gainParameter = nullptr;
    float previousGain = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrgxAudioProcessor)
};
