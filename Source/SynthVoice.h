#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

// Jedan glas syntha: svira jednu notu odjednom (pilasti val).
// Synthesiser drži više ovakvih glasova, pa možemo svirati akorde.
class SynthVoice : public juce::SynthesiserVoice
{
public:
    // Glas čita ovojnicu (attack, decay, sustain, release) izravno iz parametara plugina.
    explicit SynthVoice (juce::AudioProcessorValueTreeState& state);

    bool canPlaySound (juce::SynthesiserSound*) override;

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int) override {}
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
    double phaseIncrement = 0.0;  // za koliko se faza pomakne po uzorku (ovisi o visini note)
    float level = 0.0f;           // jačina note (koliko jako je tipka pritisnuta)

    // Pokazivači na gumbe ovojnice. Uzimamo ih jednom, u konstruktoru, a ne na audio niti.
    std::atomic<float>* attackParameter;
    std::atomic<float>* decayParameter;
    std::atomic<float>* sustainParameter;
    std::atomic<float>* releaseParameter;

    juce::ADSR envelope;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthVoice)
};
