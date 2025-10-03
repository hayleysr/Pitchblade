// austin n reyna

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"

//gain panel obj
class NoiseGatePanel : public juce::Component
    //private juce::Slider::Listener
{
public:
    explicit NoiseGatePanel(AudioPluginAudioProcessor& proc);
    void resized() override;
    void paint(juce::Graphics&) override;

private:
    //void sliderValueChanged(juce::Slider* slider) override;
    // Reference back to main processor
    AudioPluginAudioProcessor& processor;

    // Sliders for noise gate
    juce::Slider thresholdSlider, attackSlider, releaseSlider;

    // Labels
    juce::Label noiseGateLabel, thresholdLabel, attackLabel, releaseLabel;

	// attachments to link sliders to the apvts parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
};