cmake_minimum_required(VERSION 3.22)

project(RG_G4 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
)

FetchContent_MakeAvailable(JUCE)

juce_add_plugin(RG_G4
    COMPANY_NAME "RG_Audio"
    COMPANY_WEBSITE "https://github.com/fodpedal1-cmyk/RG-G4-distortion-pedal"
    COMPANY_EMAIL "rg.audio@example.com"

    PLUGIN_MANUFACTURER_CODE RGAu
    PLUGIN_CODE RgG4

    FORMATS VST3

    PRODUCT_NAME "RG G4"
    PLUGIN_NAME "RG G4"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE

    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(RG_G4
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_link_libraries(RG_G4
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_dsp
        juce::juce_gui_basics
)

target_compile_definitions(RG_G4
    PRIVATE
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

set_target_properties(RG_G4 PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)
