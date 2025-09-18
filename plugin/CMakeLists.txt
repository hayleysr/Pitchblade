# Specify minimum CMake version supported. Project tested on 4.1.1
cmake_minimum_required(VERSION 3.22)

# Project name
project(Pitchblade VERSION 0.1.0)

# Plugin configurtion data
juce_add_plugin(${PROJECT_NAME}
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME "WSU PITCHBLADE TEAM"          # Specify the name of the plugin's author
    IS_SYNTH FALSE                              # Is this a synth or an effect?
    NEEDS_MIDI_INPUT FALSE                      # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT FALSE                     # Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                        # Is this plugin a MIDI effect?
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE            # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE Pbld               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Pbld                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS AU VST3 Standalone                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "Pitchblade"                  # The name of the final executable, which can differ from the target name
)

target_sources(${PROJECT_NAME}
    PRIVATE
        source/PluginEditor.cpp
        source/PluginProcessor.cpp
        source/GainProcessor.cpp
        source/NoiseGateProcessor.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include
)

#Include these libraries in build
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags     # Config flags
        juce::juce_recommended_lto_flags        # Link time optimization flags
        juce::juce_recommended_warning_flags    # Warning flags
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

if(MSVC)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
        _CRT_SECURE_NO_WARNINGS
)
endif()