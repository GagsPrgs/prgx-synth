#pragma once

#include "Filter.h"
#include <juce_audio_utils/juce_audio_utils.h>

// Jedan glas syntha: svira jednu notu odjednom.
// Zvuk prolazi ovim putem: oscilator -> filtar -> ovojnica (jačina kroz vrijeme).
// Synthesiser drži više ovakvih glasova, pa možemo svirati akorde.
class SynthVoice : public juce::SynthesiserVoice
{
public:
    // Glas čita gumbe (valni oblik, filtar, ovojnicu) izravno iz parametara plugina.
    explicit SynthVoice (juce::AudioProcessorValueTreeState& state);

    bool canPlaySound (juce::SynthesiserSound*) override;

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int, int) override {}

    void setCurrentPlaybackSampleRate (double newRate) override;

    // Poziva se na audio niti. Vidi pravila u CLAUDE.md.
    void renderNextBlock (juce::AudioBuffer<float>&, int startSample, int numSamples) override;
    using SynthesiserVoice::renderNextBlock;

private:
    juce::ADSR::Parameters readEnvelopeParameters() const;
    void updateEnvelope();

    // Glasnoća jednog glasa. Mala je da se nekoliko nota može zbrojiti bez pucanja.
    static constexpr float voiceGain = 0.3f;

    double phase = 0.0;           // gdje smo u jednom titraju vala (od 0 do 1)
    double baseIncrement = 0.0;   // za koliko se faza pomakne po uzorku (ovisi o visini note)
    double phaseIncrement = 0.0;  // isto, ali s uračunatim pitch bendom
    double bendSemitones = 2.0;   // za koliko poluglasa kotačić pomiče notu (gore i dolje)
    float level = 0.0f;           // jačina note (koliko jako je tipka pritisnuta)

    // Glatki prijelazi, da pomicanje kotačića ili gumba ne "štuca" u zvuku.
    juce::SmoothedValue<double> smoothedPitchBend;                                   // od -1 do +1
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> smoothedCutoff;  // u Hz
    juce::SmoothedValue<float> smoothedFilterMix;                                    // 0 = mimo filtra, 1 = kroz filtar

    LowPassFilter filter;
    float appliedResonance = -1.0f;  // -1 znači "filtar još nije podešen za ovu notu"

    // Pokazivači na gumbe. Uzimamo ih jednom, u konstruktoru, a ne na audio niti.
    std::atomic<float>* waveformParameter;
    std::atomic<float>* bendRangeParameter;
    std::atomic<float>* cutoffParameter;
    std::atomic<float>* resonanceParameter;
    std::atomic<float>* attackParameter;
    std::atomic<float>* decayParameter;
    std::atomic<float>* sustainParameter;
    std::atomic<float>* releaseParameter;

    juce::ADSR envelope;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthVoice)
};
