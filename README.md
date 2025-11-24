# Pitchblade

***Pitchblade*** is a vocal multi-tool audio plugin built with C++23 and the JUCE framework. It is designed for comprehensive vocal processing, and it combines many tools into a singular, modular interface.

The plugin features a dynamic "Daisy Chain" signal flow, allowing users to reorder effects on the fly, alongside real-time visualizers for precise monitoring.

## Features

Pitchblade includes several processing modules tailored for vocals:
* **Pitch Correction and Shifting:** Real-time pitch detection and correction (powered by RubberBand)
* **Formant Manipulation:** Formant detection and shifting for altering vocal timbre without affecting pitch.
* **Gain:** Increase or decrease volume.
* **Compressor:** Controls dynamic range, making louder sounds quieter. Also includes a limiter.
* **Noise Gate:** Eliminates sounds below a certain level.
* **De-Esser:** Reduces harsh sibilance.
* **De-Noiser:** Learns what sounds are present in the background to reduce them. Recommended to be used with the Noise Gate for full noise reduction.
* **Equalizer:** Increases or reduces amplitude at specific frequencies to shape the sound of a voice.

Pitchblade also includes a few quality-of-life features to make the user experience better:
* **Visualizers:** Each panel has its own specialized visualizer based on two visualizer template classes.
* **Preset Management:** Save and load custom effect chains.

## Prerequisites

Before building, ensure you have the following installed:
* **CMake:** Version 3.22 or higher
* **C++ Compiler:** Must support **C++23** (e.g., Visual Studio 2022 v17.x on Windows).
* **Git:** For version control.
* *Note: JUCE and GoogleTest dependencies are automatically handled via the specific CMake configuration.*

## Build Instructions

### Windows

You can build the project using the provided helper script.

To generate the project files and build the solution completely (Debug mode):
```bat
.\configure_windows.bat
```

## Test Case Instructions

After building, you can run the various test cases included in ./tests by running the provided helper script.
```bat
.\configure_windows_test.bat
```