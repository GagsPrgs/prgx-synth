#pragma once

#include "LabeledKnob.h"
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
    LabeledKnob attackKnob, decayKnob, sustainKnob, releaseKnob, volumeKnob;

    // Pravokutnici naslova i dviju kartica. Računaju se u resized(), a crtaju u paint().
    juce::Rectangle<int> titleArea, envelopePanel, outputPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrgxAudioProcessorEditor)
};
