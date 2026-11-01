#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

// Crtež oblika ovojnice (Attack, Decay, Sustain, Release). Mijenja se dok vrtiš gumbe ili
// kad FL Studio automatizira parametre. Samo prikazuje, ništa ne mijenja.
class EnvelopeDisplay : public juce::Component
{
public:
    explicit EnvelopeDisplay (juce::AudioProcessorValueTreeState& state);

    void paint (juce::Graphics&) override;

private:
    // Jedan parametar ovojnice: pamti položaj gumba (od 0 do 1) i traži ponovno crtanje kad se promijeni.
    struct Stage
    {
        Stage (EnvelopeDisplay& owner, juce::RangedAudioParameter& parameter);

        float position = 0.0f;
        juce::ParameterAttachment attachment;
    };

    Stage attack, decay, sustain, release;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeDisplay)
};
