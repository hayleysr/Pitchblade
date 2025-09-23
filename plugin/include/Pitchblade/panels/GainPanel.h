// reyna macabebe

// austin


#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
//gain panel
class GainPanel : public juce::Component,
    private juce::Slider::Listener
{
public:
    explicit GainPanel(AudioPluginAudioProcessor& proc);
    void resized() override;

private:
    void sliderValueChanged(juce::Slider* slider) override;
    //declaring slider n processor
    AudioPluginAudioProcessor& processor;
    juce::Slider gainSlider;

    //Label - Austin
    juce::Label gainLabel;
};
