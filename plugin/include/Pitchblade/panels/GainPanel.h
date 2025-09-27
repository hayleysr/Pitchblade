// austin and reyna


#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
//gain panel
class GainPanel : public juce::Component
    //private juce::Slider::Listener
{
public:
    explicit GainPanel(AudioPluginAudioProcessor& proc);
    void resized() override;
    void paint(juce::Graphics&) override; 

private:
    //void sliderValueChanged(juce::Slider* slider) override;
    //declaring slider n processor
    AudioPluginAudioProcessor& processor;
    juce::Slider gainSlider;

    //Label - Austin
    juce::Label gainLabel;

	// attachment to link slider to the apvts parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
};
