#include "PluginEditor.h"
#include "ParameterIds.h"
#include "Theme.h"

namespace
{
    constexpr int margin = 16;
    constexpr int titleHeight = 56;
    constexpr int panelPadding = 12;
    constexpr int panelHeaderHeight = 28;
    constexpr int panelGap = 12;
    constexpr int knobColumnWidth = 112;
    constexpr int knobHeight = 176;  // natpis + gumb + vrijednost

    constexpr int envelopePanelWidth = 4 * knobColumnWidth + 2 * panelPadding;
    constexpr int outputPanelWidth = knobColumnWidth + 2 * panelPadding;
    constexpr int panelHeight = panelHeaderHeight + knobHeight + 2 * panelPadding;

    constexpr int windowWidth = 2 * margin + envelopePanelWidth + panelGap + outputPanelWidth;
    constexpr int windowHeight = 2 * margin + titleHeight + panelHeight;

    // Kartica s tamnijim okvirom i malim naslovom u gornjem lijevom kutu.
    void drawPanel (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title)
    {
        g.setColour (Theme::panel);
        g.fillRoundedRectangle (bounds.toFloat(), 8.0f);

        g.setColour (Theme::panelOutline);
        g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f), 8.0f, 1.0f);

        g.setColour (Theme::dimText);
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        g.drawText (title, bounds.reduced (panelPadding + 4, 0).removeFromTop (panelHeaderHeight + panelPadding),
                    juce::Justification::centredLeft);
    }
}

PrgxAudioProcessorEditor::PrgxAudioProcessorEditor (PrgxAudioProcessor& p)
    : AudioProcessorEditor (&p),
      attackKnob (p.parameters, ParameterIds::attack, "Attack"),
      decayKnob (p.parameters, ParameterIds::decay, "Decay"),
      sustainKnob (p.parameters, ParameterIds::sustain, "Sustain"),
      releaseKnob (p.parameters, ParameterIds::release, "Release"),
      volumeKnob (p.parameters, ParameterIds::gain, "Volume")
{
    for (auto* knob : { &attackKnob, &decayKnob, &sustainKnob, &releaseKnob, &volumeKnob })
        addAndMakeVisible (knob);

    setSize (windowWidth, windowHeight);
}

void PrgxAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::background);

    g.setColour (Theme::text);
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("PRGX Synth", titleArea.withTrimmedLeft (4), juce::Justification::centredLeft);

    drawPanel (g, envelopePanel, "ENVELOPE");
    drawPanel (g, outputPanel, "OUTPUT");
}

void PrgxAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (margin);
    titleArea = area.removeFromTop (titleHeight);

    envelopePanel = area.removeFromLeft (envelopePanelWidth);
    area.removeFromLeft (panelGap);
    outputPanel = area;

    auto envelopeKnobs = envelopePanel.reduced (panelPadding);
    envelopeKnobs.removeFromTop (panelHeaderHeight);

    attackKnob.setBounds (envelopeKnobs.removeFromLeft (knobColumnWidth));
    decayKnob.setBounds (envelopeKnobs.removeFromLeft (knobColumnWidth));
    sustainKnob.setBounds (envelopeKnobs.removeFromLeft (knobColumnWidth));
    releaseKnob.setBounds (envelopeKnobs.removeFromLeft (knobColumnWidth));

    auto outputKnobs = outputPanel.reduced (panelPadding);
    outputKnobs.removeFromTop (panelHeaderHeight);
    volumeKnob.setBounds (outputKnobs);
}
