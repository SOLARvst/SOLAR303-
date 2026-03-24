cmake_minimum_required(VERSION 3.22)
project(SOLAR303 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download JUCE 7
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.9
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(SOLAR303
    VERSION                   1.0.0
    COMPANY_NAME              "SOLAR"
    BUNDLE_ID                 "com.solar.solar303"
    PLUGIN_MANUFACTURER_CODE  Solr
    PLUGIN_CODE               So03
    FORMATS                   VST3 AU Standalone
    PRODUCT_NAME              "SOLAR 303"
    IS_SYNTH                  TRUE
    NEEDS_MIDI_INPUT          TRUE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD   FALSE
)

juce_generate_juce_header(SOLAR303)

# ── Embedded fonts ────────────────────────────────────────────────────────────
juce_add_binary_data(SOLAR303_Fonts
    SOURCES
        Source/Fonts/EasyRideExpand.ttf
        Source/Fonts/aAcakadut.ttf
)

target_sources(SOLAR303
    PRIVATE
        Source/DiodeLadderFilter.cpp
        Source/SOLAR303Engine.cpp
        Source/SOLAR303Sequencer.cpp
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(SOLAR303
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(SOLAR303
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
        SOLAR303_Fonts
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
