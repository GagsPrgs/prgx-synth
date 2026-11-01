#include "EnvelopeDisplay.h"
#include "ParameterIds.h"
#include "Theme.h"

EnvelopeDisplay::Stage::Stage (EnvelopeDisplay& owner, juce::RangedAudioParameter& parameter)
    : attachment (parameter, [this, &owner, &parameter] (float value)
                  {
                      position = parameter.convertTo0to1 (value);
                      owner.repaint();
                  })
{
    attachment.sendInitialUpdate();
}

EnvelopeDisplay::EnvelopeDisplay (juce::AudioProcessorValueTreeState& state)
    : attack (*this, *state.getParameter (ParameterIds::attack)),
      decay (*this, *state.getParameter (ParameterIds::decay)),
      sustain (*this, *state.getParameter (ParameterIds::sustain)),
      release (*this, *state.getParameter (ParameterIds::release))
{
}

void EnvelopeDisplay::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    Theme::fillDisplay (g, bounds);

    const auto area = bounds.reduced (14.0f, 12.0f);

    // Širina dijela krivulje ovisi o položaju njegova gumba (ne o točnoj sekundi), pa se
    // i vrlo kratka i vrlo duga vremena jasno vide. Sustain je ravni dio fiksne širine.
    const float holdWidth = area.getWidth() * 0.2f;
    const float maxStageWidth = (area.getWidth() - holdWidth) / 3.0f;
    const auto stageWidth = [maxStageWidth] (float position) { return maxStageWidth * (0.1f + 0.9f * position); };

    const float x0 = area.getX();
    const float x1 = x0 + stageWidth (attack.position);
    const float x2 = x1 + stageWidth (decay.position);
    const float x3 = x2 + holdWidth;
    const float x4 = x3 + stageWidth (release.position);

    const float top = area.getY();
    const float bottom = area.getBottom();
    const float sustainY = bottom - sustain.position * area.getHeight();

    // JUCE-ov ADSR mijenja razinu pravocrtno, pa je i crtež sastavljen od ravnih dijelova.
    juce::Path curve;
    curve.startNewSubPath (x0, bottom);
    curve.lineTo (x1, top);
    curve.lineTo (x2, sustainY);
    curve.lineTo (x3, sustainY);
    curve.lineTo (x4, bottom);

    juce::Path filled (curve);
    filled.closeSubPath();

    g.setColour (Theme::accent.withAlpha (0.18f));
    g.fillPath (filled);

    g.setColour (Theme::accent);
    g.strokePath (curve, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const juce::Point<float> breakpoints[] { { x1, top }, { x2, sustainY }, { x3, sustainY } };

    for (auto point : breakpoints)
        g.fillEllipse (point.x - 3.0f, point.y - 3.0f, 6.0f, 6.0f);
}
