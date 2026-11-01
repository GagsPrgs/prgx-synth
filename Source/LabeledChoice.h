#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

// Padajući izbornik s natpisom iznad, spojen na parametar s nekoliko ponuđenih izbora.
class LabeledChoice : public juce::Component
{
public:
    LabeledChoice (juce::AudioProcessorValueTreeState& state, const juce::String& parameterId, const juce::String& labelText);

    void resized() override;

    // Visina koju komponenta treba (natpis + izbornik).
    static constexpr int preferredHeight = 24 + 4 + 28;

private:
    juce::Label label;
    juce::ComboBox box;

    // Povezuje izbornik s parametrom u oba smjera. Napravi se tek kad izbornik ima sve izbore,
    // a deklariran je nakon njega jer se mora uništiti prije njega.
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabeledChoice)
};
