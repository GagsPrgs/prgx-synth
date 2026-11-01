#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

// Crtež odabranog valnog oblika (dva titraja). Samo prikazuje, ništa ne mijenja.
class WaveformDisplay : public juce::Component
{
public:
    explicit WaveformDisplay (juce::AudioProcessorValueTreeState& state);

    void paint (juce::Graphics&) override;

private:
    int waveform = 1;  // isti redoslijed kao popis valnih oblika u parametru

    juce::ParameterAttachment attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
