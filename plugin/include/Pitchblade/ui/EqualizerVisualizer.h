// EQ response visualizer using FrequencyGraphVisualizer
#pragma once

#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/ui/FrequencyGraphVisualizer.h"

// Renders the static EQ frequency response curve for the current Equalizer settings
class EqualizerVisualizer : public juce::Component, private juce::Timer {
public:
    explicit EqualizerVisualizer(AudioPluginAudioProcessor& proc);
    ~EqualizerVisualizer() override;

    void resized() override;

private:
    void timerCallback() override;
    void updateResponseCurve();

    AudioPluginAudioProcessor& processor;
    std::unique_ptr<FrequencyGraphVisualizer> graph; // draws frequency vs dB

    // Helper to build log-spaced frequency array between 20 Hz and 20 kHz
    static void buildLogFrequencies(std::vector<float>& freqs, int numPoints);
};
