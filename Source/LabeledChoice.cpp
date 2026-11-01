#include "LabeledChoice.h"
#include "Theme.h"

LabeledChoice::LabeledChoice (juce::AudioProcessorValueTreeState& state, const juce::String& parameterId, const juce::String& labelText)
{
    // Izbornik mora imati sve izbore prije nego što ga spojimo s parametrom.
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (state.getParameter (parameterId)))
        box.addItemList (choice->choices, 1);

    box.setJustificationType (juce::Justification::centred);
    box.setColour (juce::ComboBox::backgroundColourId, Theme::background);
    box.setColour (juce::ComboBox::textColourId, Theme::text);
    box.setColour (juce::ComboBox::outlineColourId, Theme::panelOutline);
    box.setColour (juce::ComboBox::arrowColourId, Theme::accent);
    addAndMakeVisible (box);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, parameterId, box);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::FontOptions (15.0f));
    label.setColour (juce::Label::textColourId, Theme::text);
    addAndMakeVisible (label);
}

void LabeledChoice::resized()
{
    auto area = getLocalBounds();
    label.setBounds (area.removeFromTop (24));
    area.removeFromTop (4);
    box.setBounds (area.removeFromTop (28));
}
