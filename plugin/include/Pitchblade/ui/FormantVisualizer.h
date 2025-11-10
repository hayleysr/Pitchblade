#pragma once

#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/FrequencyGraphVisualizer.h"

//Author: huda
// Visualizes detected formant frequencies as vertical markers over a log-frequency axis.
// Pulls latest formants from the processor and repaints at the global framerate.
class FormantVisualizer : public juce::Component,
                          private juce::Timer,
                          private juce::AudioProcessorValueTreeState::Listener
{
public:
    FormantVisualizer(AudioPluginAudioProcessor& processorRef,
                      juce::AudioProcessorValueTreeState& vts);
    ~FormantVisualizer() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // Timer to drive UI refreshes based on GLOBAL_FRAMERATE
    void timerCallback() override;

    // Listen to GLOBAL_FRAMERATE changes
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    // Data/Config
    AudioPluginAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& apvts;

    // Child components
    std::unique_ptr<FrequencyGraphVisualizer> freqGraph; // background grid/axes

    // Overlay draws the formant markers on top of the grid
    class FormantOverlay : public juce::Component
    {
    public:
        explicit FormantOverlay(AudioPluginAudioProcessor& procRef)
            : proc(procRef) {}

        void paint(juce::Graphics& g) override;

        // Align with FrequencyGraphVisualizer layout (left and bottom label areas)
        static constexpr int labelWidth  = 40;
        static constexpr int labelHeight = 20;

    private:
        AudioPluginAudioProcessor& proc;
        // Underlying FrequencyGraphVisualizer uses ~20..20000 Hz log scale.
        const juce::Range<float> baseXAxisHz { 20.0f, 20000.0f };
        // Visible window tailored to detected formants.
        const juce::Range<float> visibleXAxisHz { 300.0f, 5000.0f };
        float logBaseStart    = std::log10(baseXAxisHz.getStart());
        float logBaseEnd      = std::log10(baseXAxisHz.getEnd());
        float logVisibleStart = std::log10(visibleXAxisHz.getStart());
        float logVisibleEnd   = std::log10(visibleXAxisHz.getEnd());

        float mapBaseFreqToX(float freq, juce::Rectangle<int> graph) const;
        float mapVisibleFreqToX(float freq, juce::Rectangle<int> graph) const;
    };

    std::unique_ptr<FormantOverlay> overlay;
};
