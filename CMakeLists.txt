cmake_minimum_required(VERSION 3.22)

project(RG_Precision_Drive VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
)

FetchContent_MakeAvailable(JUCE)

#==============================================================
# RG PRECISION DRIVE
#==============================================================

juce_add_plugin(RG_Precision_Drive
    COMPANY_NAME "RG Electronics"
    PLUGIN_MANUFACTURER_CODE Rged
    PLUGIN_CODE RPD1
    FORMATS VST3
    PRODUCT_NAME "RG Precision Drive"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE

    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

#==============================================================
# GENERATE JUCE HEADER
#==============================================================

juce_generate_juce_header(RG_Precision_Drive)

#==============================================================
# SOURCE
#==============================================================

target_sources(RG_Precision_Drive
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_include_directories(RG_Precision_Drive
    PRIVATE
        Source
)

#==============================================================
# JUCE MODULES
#==============================================================

target_link_libraries(RG_Precision_Drive
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp

    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

#==============================================================
# DEFINITIONS
#==============================================================

target_compile_definitions(RG_Precision_Drive
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
