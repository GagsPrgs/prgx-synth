#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

// Boje sučelja na jednom mjestu.
namespace Theme
{
    inline const juce::Colour background { 0xff1b1b21 };
    inline const juce::Colour panel { 0xff25252d };
    inline const juce::Colour panelOutline { 0xff35353f };
    inline const juce::Colour accent { 0xff4fc3f7 };
    inline const juce::Colour text { 0xffe8e8ee };
    inline const juce::Colour dimText { 0xff9a9aa8 };

    // Tamna "udubina" u kojoj se crtaju krivulje (valni oblik, filtar, ovojnica).
    inline void fillDisplay (juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour (background);
        g.fillRoundedRectangle (bounds, 6.0f);
    }
}
