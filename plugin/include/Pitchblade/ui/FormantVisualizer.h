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
        const juce::Range<float> xAxisHz { 20.0f, 5000.0f };
        float logFreqStart = std::log10(xAxisHz.getStart());
        float logFreqEnd   = std::log10(xAxisHz.getEnd());

        float mapFreqToX(float freq, juce::Rectangle<int> graph) const;
    };

    std::unique_ptr<FormantOverlay> overlay;
};
