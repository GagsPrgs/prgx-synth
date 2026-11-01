#include "FilterDisplay.h"
#include "Filter.h"
#include "ParameterIds.h"
#include "Theme.h"

namespace
{
    constexpr double minHz = 20.0;
    constexpr double maxHz = 20000.0;
    constexpr double minDb = -36.0;
    constexpr double maxDb = 24.0;
    constexpr double drawingSampleRate = 48000.0;  // odziv se crta kao da je brzina uzorkovanja 48 kHz
}

FilterDisplay::FilterDisplay (juce::AudioProcessorValueTreeState& state)
    : cutoffAttachment (*state.getParameter (ParameterIds::cutoff), [this] (float value)
                        {
                            cutoff = value;
                            repaint();
                        }),
      resonanceAttachment (*state.getParameter (ParameterIds::resonance), [this] (float value)
                           {
                               resonance = value;
                               repaint();
                           })
{
    cutoffAttachment.sendInitialUpdate();
    resonanceAttachment.sendInitialUpdate();
}

void FilterDisplay::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    Theme::fillDisplay (g, bounds);

    const auto area = bounds.reduced (14.0f, 12.0f);

    // Frekvencija ide logaritamski (zdesna se udvostručuje jednako brzo), a razina je u decibelima.
    const auto xFor = [&] (double hz)
    {
        return area.getX() + (float) (std::log (hz / minHz) / std::log (maxHz / minHz)) * area.getWidth();
    };

    const auto yFor = [&] (double db)
    {
        const double proportion = juce::jlimit (0.0, 1.0, (db - minDb) / (maxDb - minDb));
        return area.getBottom() - (float) proportion * area.getHeight();
    };

    // Crta nula decibela (razina bez filtra) i mjesto cutoffa.
    g.setColour (Theme::panelOutline);
    g.drawHorizontalLine ((int) yFor (0.0), area.getX(), area.getRight());
    g.drawVerticalLine ((int) xFor (cutoff), area.getY(), area.getBottom());

    juce::Path response;
    const int numPoints = juce::jmax (2, (int) area.getWidth());

    for (int i = 0; i < numPoints; ++i)
    {
        const double hz = minHz * std::pow (maxHz / minHz, (double) i / (numPoints - 1));
        const double gain = LowPassFilter::isOpen (cutoff, resonance) ? 1.0
                                                                       : LowPassFilter::magnitudeAt (hz, cutoff, resonance, drawingSampleRate);
        const double db = 20.0 * std::log10 (gain);
        const float x = area.getX() + area.getWidth() * (float) i / (numPoints - 1);

        if (i == 0)
            response.startNewSubPath (x, yFor (db));
        else
            response.lineTo (x, yFor (db));
    }

    juce::Path filled (response);
    filled.lineTo (area.getRight(), area.getBottom());
    filled.lineTo (area.getX(), area.getBottom());
    filled.closeSubPath();

    g.setColour (Theme::accent.withAlpha (0.18f));
    g.fillPath (filled);

    g.setColour (Theme::accent);
    g.strokePath (response, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}
