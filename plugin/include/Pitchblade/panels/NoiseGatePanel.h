// austin 

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"

//gain panel obj
class NoiseGatePanel : public juce::Component,
    private juce::Slider::Listener
{
public:
    explicit NoiseGatePanel(AudioPluginAudioProcessor& proc);
    void resized() override;
    void paint(juce::Graphics&) override;

private:
    void sliderValueChanged(juce::Slider* slider) override;
    // Reference back to main processor
    AudioPluginAudioProcessor& processor;

    // Sliders for noise gate
    juce::Slider thresholdSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;

    // Labels
    juce::Label noiseGateLabel;
    juce::Label thresholdLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
};