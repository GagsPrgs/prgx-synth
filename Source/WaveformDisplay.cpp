#include "WaveformDisplay.h"
#include "ParameterIds.h"
#include "Theme.h"

namespace
{
    // Vrijednost vala od -1 do +1 za fazu od 0 do 1 (isti oblici kao u SynthVoice, samo za crtanje).
    float waveformValue (int waveform, float phase)
    {
        switch (waveform)
        {
            case 0:  return std::sin (juce::MathConstants<float>::twoPi * phase);
            case 2:  return phase < 0.5f ? 1.0f : -1.0f;
            case 3:  return 4.0f * std::abs (std::fmod (phase + 0.75f, 1.0f) - 0.5f) - 1.0f;
            default: return 2.0f * phase - 1.0f;
        }
    }
}

WaveformDisplay::WaveformDisplay (juce::AudioProcessorValueTreeState& state)
    : attachment (*state.getParameter (ParameterIds::waveform), [this] (float value)
                  {
                      waveform = juce::roundToInt (value);
                      repaint();
                  })
{
    attachment.sendInitialUpdate();
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    Theme::fillDisplay (g, bounds);

    const auto area = bounds.reduced (14.0f, 14.0f);
    constexpr int numCycles = 2;
    constexpr int pointsPerCycle = 64;

    g.setColour (Theme::panelOutline);
    g.drawHorizontalLine ((int) area.getCentreY(), area.getX(), area.getRight());

    juce::Path wave;

    for (int i = 0; i <= numCycles * pointsPerCycle; ++i)
    {
        const float position = (float) i / (numCycles * pointsPerCycle);
        const float phase = std::fmod ((float) i / pointsPerCycle, 1.0f);
        const float x = area.getX() + position * area.getWidth();
        const float y = area.getCentreY() - waveformValue (waveform, phase) * area.getHeight() * 0.5f;

        if (i == 0)
            wave.startNewSubPath (x, y);
        else
            wave.lineTo (x, y);
    }

    g.setColour (Theme::accent);
    g.strokePath (wave, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}
