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

    constexpr int knobColumnWidth = 96;
    constexpr int knobHeight = 150;     // natpis + gumb + vrijednost
    constexpr int displayHeight = 90;   // crtež pri vrhu kartice (valni oblik, filtar, ovojnica)
    constexpr int displayGap = 8;
    constexpr int choiceGap = 10;       // razmak između dva izbornika u kartici OSCILLATOR

    constexpr int oscillatorPanelWidth = 144;
    constexpr int filterPanelWidth = 2 * knobColumnWidth + 2 * panelPadding;
    constexpr int envelopePanelWidth = 4 * knobColumnWidth + 2 * panelPadding;
    constexpr int outputPanelWidth = 144;

    constexpr int panelHeight = panelHeaderHeight + displayHeight + displayGap + knobHeight + 2 * panelPadding;

    constexpr int windowWidth = 2 * margin + oscillatorPanelWidth + filterPanelWidth + envelopePanelWidth
                                + outputPanelWidth + 3 * panelGap;
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

    // Prostor kartice ispod naslova.
    juce::Rectangle<int> contentOf (juce::Rectangle<int> panel)
    {
        auto content = panel.reduced (panelPadding);
        content.removeFromTop (panelHeaderHeight);
        return content;
    }
}

PrgxAudioProcessorEditor::PrgxAudioProcessorEditor (PrgxAudioProcessor& p)
    : AudioProcessorEditor (&p),
      waveformDisplay (p.parameters),
      waveformChoice (p.parameters, ParameterIds::waveform, "Waveform"),
      bendRangeChoice (p.parameters, ParameterIds::bendRange, "Bend Range"),
      filterDisplay (p.parameters),
      cutoffKnob (p.parameters, ParameterIds::cutoff, "Cutoff"),
      resonanceKnob (p.parameters, ParameterIds::resonance, "Resonance"),
      envelopeDisplay (p.parameters),
      attackKnob (p.parameters, ParameterIds::attack, "Attack"),
      decayKnob (p.parameters, ParameterIds::decay, "Decay"),
      sustainKnob (p.parameters, ParameterIds::sustain, "Sustain"),
      releaseKnob (p.parameters, ParameterIds::release, "Release"),
      volumeKnob (p.parameters, ParameterIds::gain, "Volume")
{
    for (juce::Component* child : std::initializer_list<juce::Component*> {
             &waveformDisplay, &waveformChoice, &bendRangeChoice, &filterDisplay, &cutoffKnob, &resonanceKnob, &envelopeDisplay,
             &attackKnob, &decayKnob, &sustainKnob, &releaseKnob, &volumeKnob })
        addAndMakeVisible (child);

    setSize (windowWidth, windowHeight);
}

void PrgxAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::background);

    g.setColour (Theme::text);
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("PRGX Synth", titleArea.withTrimmedLeft (4), juce::Justification::centredLeft);

    drawPanel (g, oscillatorPanel, "OSCILLATOR");
    drawPanel (g, filterPanel, "FILTER");
    drawPanel (g, envelopePanel, "ENVELOPE");
    drawPanel (g, outputPanel, "OUTPUT");
}

void PrgxAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (margin);
    titleArea = area.removeFromTop (titleHeight);

    oscillatorPanel = area.removeFromLeft (oscillatorPanelWidth);
    area.removeFromLeft (panelGap);
    filterPanel = area.removeFromLeft (filterPanelWidth);
    area.removeFromLeft (panelGap);
    envelopePanel = area.removeFromLeft (envelopePanelWidth);
    area.removeFromLeft (panelGap);
    outputPanel = area;

    // Kartice imaju isti raspored: crtež pri vrhu, a gumbi u retku pri dnu. Zato su gumbi
    // u svim karticama poravnati kroz cijeli prozor.
    auto oscillatorContent = contentOf (oscillatorPanel);
    waveformDisplay.setBounds (oscillatorContent.removeFromTop (displayHeight));
    auto oscillatorRow = oscillatorContent.removeFromBottom (knobHeight);
    auto choices = oscillatorRow.withSizeKeepingCentre (oscillatorRow.getWidth(), 2 * LabeledChoice::preferredHeight + choiceGap);
    waveformChoice.setBounds (choices.removeFromTop (LabeledChoice::preferredHeight));
    choices.removeFromTop (choiceGap);
    bendRangeChoice.setBounds (choices);

    auto filterContent = contentOf (filterPanel);
    filterDisplay.setBounds (filterContent.removeFromTop (displayHeight));
    auto filterRow = filterContent.removeFromBottom (knobHeight);
    cutoffKnob.setBounds (filterRow.removeFromLeft (knobColumnWidth));
    resonanceKnob.setBounds (filterRow.removeFromLeft (knobColumnWidth));

    auto envelopeContent = contentOf (envelopePanel);
    envelopeDisplay.setBounds (envelopeContent.removeFromTop (displayHeight));
    auto envelopeRow = envelopeContent.removeFromBottom (knobHeight);
    attackKnob.setBounds (envelopeRow.removeFromLeft (knobColumnWidth));
    decayKnob.setBounds (envelopeRow.removeFromLeft (knobColumnWidth));
    sustainKnob.setBounds (envelopeRow.removeFromLeft (knobColumnWidth));
    releaseKnob.setBounds (envelopeRow.removeFromLeft (knobColumnWidth));

    // Glavna glasnoća ima veliki gumb koji popunjava cijelu karticu.
    volumeKnob.setBounds (contentOf (outputPanel));
}
