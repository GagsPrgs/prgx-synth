#include "LabeledKnob.h"
#include "Theme.h"

LabeledKnob::LabeledKnob (juce::AudioProcessorValueTreeState& state, const juce::String& parameterId, const juce::String& labelText)
    : attachment (state, parameterId, knob)
{
    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 22);
    knob.setColour (juce::Slider::rotarySliderFillColourId, Theme::accent);
    knob.setColour (juce::Slider::rotarySliderOutlineColourId, Theme::panelOutline);
    knob.setColour (juce::Slider::thumbColourId, Theme::text);
    knob.setColour (juce::Slider::textBoxTextColourId, Theme::text);
    knob.setColour (juce::Slider::textBoxOutlineColourId, Theme::panelOutline);
    addAndMakeVisible (knob);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::FontOptions (15.0f));
    label.setColour (juce::Label::textColourId, Theme::text);
    addAndMakeVisible (label);
}

void LabeledKnob::resized()
{
    auto area = getLocalBounds();
    label.setBounds (area.removeFromTop (24));
    knob.setBounds (area);
}
