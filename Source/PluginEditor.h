#pragma once

#include "PluginProcessor.h"

// Prozor plugina koji vidiš u FL Studiju.
class PrgxAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PrgxAudioProcessorEditor (PrgxAudioProcessor&);
    ~PrgxAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider volumeKnob;
    juce::Label volumeLabel;

    // Povezuje gumb s parametrom "gain" u oba smjera (gumb <-> FL Studio automatizacija).
    juce::AudioProcessorValueTreeState::SliderAttachment volumeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrgxAudioProcessorEditor)
};
