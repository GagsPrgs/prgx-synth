#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

// "Zvuk" koji synth zna svirati. Imamo samo jedan i vrijedi za sve note i sve MIDI kanale.
struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};
