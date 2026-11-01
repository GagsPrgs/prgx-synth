#pragma once

#include "EnvelopeDisplay.h"
#include "FilterDisplay.h"
#include "LabeledChoice.h"
#include "LabeledKnob.h"
#include "PluginProcessor.h"
#include "WaveformDisplay.h"

// Prozor plugina koji vidiš u FL Studiju.
// Kartice idu redoslijedom kojim zvuk putuje: oscilator -> filtar -> ovojnica -> izlaz.
class PrgxAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PrgxAudioProcessorEditor (PrgxAudioProcessor&);
    ~PrgxAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    WaveformDisplay waveformDisplay;
    LabeledChoice waveformChoice, bendRangeChoice;

    FilterDisplay filterDisplay;
    LabeledKnob cutoffKnob, resonanceKnob;

    EnvelopeDisplay envelopeDisplay;
    LabeledKnob attackKnob, decayKnob, sustainKnob, releaseKnob;

    LabeledKnob volumeKnob;

    // Pravokutnici naslova i kartica. Računaju se u resized(), a crtaju u paint().
    juce::Rectangle<int> titleArea, oscillatorPanel, filterPanel, envelopePanel, outputPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrgxAudioProcessorEditor)
};
