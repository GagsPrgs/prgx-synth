# Najmanja verzija CMake-a koju JUCE zahtijeva.
cmake_minimum_required(VERSION 3.22)

project(PRGX_SYNTH VERSION 0.1.0)

# --- Preuzimanje JUCE-a --------------------------------------------------------
# FetchContent pri prvom pokretanju sam skine JUCE s GitHuba u mapu build/_deps.
# Verziju (GIT_TAG) držimo fiksnom da bi se projekt uvijek gradio jednako.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# --- Definicija plugina ----------------------------------------------------------
juce_add_plugin(PrgxSynth
    COMPANY_NAME             "PRGX"
    PRODUCT_NAME             "PRGX Synth"     # ime koje se vidi u FL Studiju
    PLUGIN_MANUFACTURER_CODE Prgx             # 4 znaka, barem jedno veliko slovo
    PLUGIN_CODE              Prs1             # 4 znaka, jedinstveno za ovaj plugin
    IS_SYNTH                 TRUE             # instrument (generator), ne efekt
    NEEDS_MIDI_INPUT         TRUE             # prima note s klavijature / piano rolla
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD  FALSE            # kopiramo ručno (vidi README)
    NEEDS_WEB_BROWSER        FALSE
    NEEDS_CURL               FALSE
    FORMATS                  VST3)

target_compile_features(PrgxSynth PRIVATE cxx_std_20)

target_sources(PrgxSynth
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/SynthVoice.cpp)

target_compile_definitions(PrgxSynth
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(PrgxSynth
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
