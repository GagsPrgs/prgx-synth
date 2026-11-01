#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

// Okrugli gumb s natpisom iznad i vrijednošću ispod, spojen na jedan parametar plugina.
class LabeledKnob : public juce::Component
{
public:
    LabeledKnob (juce::AudioProcessorValueTreeState& state, const juce::String& parameterId, const juce::String& labelText);

    void resized() override;

private:
    juce::Slider knob;
    juce::Label label;

    // Povezuje gumb s parametrom u oba smjera (gumb <-> FL Studio automatizacija).
    // Mora biti deklariran nakon gumba, jer se uništava prije njega.
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabeledKnob)
};
