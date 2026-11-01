#include "PluginEditor.h"

PrgxAudioProcessorEditor::PrgxAudioProcessorEditor (PrgxAudioProcessor& p)
    : AudioProcessorEditor (&p),
      volumeAttachment (p.parameters, "gain", volumeKnob)
{
    volumeKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    volumeKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (volumeKnob);

    volumeLabel.setText ("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (volumeLabel);

    setSize (300, 300);
}

void PrgxAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1e24));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (20.0f));
    g.drawFittedText ("PRGX Synth", getLocalBounds().removeFromTop (50), juce::Justification::centred, 1);
}

void PrgxAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (40);  // mjesto za naslov

    volumeLabel.setBounds (area.removeFromTop (24));
    volumeKnob.setBounds (area);
}
