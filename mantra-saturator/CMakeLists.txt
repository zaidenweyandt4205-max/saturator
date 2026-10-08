cmake_minimum_required(VERSION 3.22)

project(MantraSaturator VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#--- JUCE ---------------------------------------------------------------------
# Pinned to a known-good release. Bump this tag to update JUCE.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.3
)
FetchContent_MakeAvailable(JUCE)

#--- Plugin -------------------------------------------------------------------
juce_add_plugin(MantraSaturator
    COMPANY_NAME              "Mantra"
    COMPANY_WEBSITE           "https://mantraent.co"
    PLUGIN_MANUFACTURER_CODE  Mntr
    PLUGIN_CODE               MSat
    FORMATS                   VST3
    PRODUCT_NAME              "Mantra Saturator"
    DESCRIPTION               "Mantra Saturator - drive, tone, mix and output. 4x oversampled tanh saturation."
    VST3_CATEGORIES           "Fx|Distortion"
)

target_sources(MantraSaturator
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_link_libraries(MantraSaturator
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(MantraSaturator
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
