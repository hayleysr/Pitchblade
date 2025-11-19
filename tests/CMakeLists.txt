# Project name for tests
project(PitchbladeTests)

# Create the test executable
add_executable(runTests
    # List all your test files here as you create them
    test_main.cpp
    test_GainProcessor.cpp
    test_NoiseGateProcessor.cpp
    # test_CompressorModule.cpp
    # test_DeEsserModule.cpp
    # test_DeNoiserModule.cpp
    # test_Visualizers.cpp
    # test_ParameterIntegration.cpp
    test_UI_DaisyChain.cpp
)

# This forces CMake to build the 'Pitchblade' target (and generate JuceHeader.h)
# *before* it tries to build 'runTests'. This fixes the "JuceHeader.h not found" error.
add_dependencies(runTests Pitchblade BinaryData)

# Link Google Test
target_link_libraries(runTests
    PRIVATE
        # gtest_main
        gtest
        Pitchblade
        BinaryData
)

# --- Link your plugin's main processor ---
target_link_libraries(runTests
    PRIVATE
        # This will bring in the main processor (Pitchblade)
        # and all its dependencies (panels, effects, etc.)
        Pitchblade

        # Linking to BinaryData (which is an INTERFACE library)
        # automatically adds the correct include path for "BinaryData.h"
        BinaryData
)

# Include directories
target_include_directories(runTests
    PRIVATE
        # We MUST use CMAKE_SOURCE_DIR, not PROJECT_SOURCE_DIR
        ${CMAKE_SOURCE_DIR}/plugin/include
        
        # This is created by JUCE and needed by PluginProcessor.h
        ${CMAKE_BINARY_DIR}/plugin/Pitchblade_artefacts/JuceLibraryCode
        
        # This path is for the auto-generated "BinaryData.h"
        ${CMAKE_BINARY_DIR}/plugin/Pitchblade_artefacts/BinaryData

        # Explicitly add the gtest header paths.
        ${gtest_SOURCE_DIR}/googletest/include
        ${gtest_SOURCE_DIR}/googlemock/include
)

# --- CTest Integration ---
# Automatically discover all tests in your executable
include(GoogleTest)
gtest_discover_tests(runTests)