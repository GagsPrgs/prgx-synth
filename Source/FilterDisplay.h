#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

// Crtež odziva filtra: koliko se koja frekvencija (od 20 Hz do 20 kHz) pojačava ili utišava.
// Mijenja se dok vrtiš Cutoff i Resonance. Samo prikazuje, ništa ne mijenja.
class FilterDisplay : public juce::Component
{
public:
    explicit FilterDisplay (juce::AudioProcessorValueTreeState& state);

    void paint (juce::Graphics&) override;

private:
    float cutoff = 20000.0f;
    float resonance = 0.0f;

    juce::ParameterAttachment cutoffAttachment, resonanceAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterDisplay)
};
