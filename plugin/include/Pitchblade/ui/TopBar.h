// reyna macabebe

#pragma once
#include <JuceHeader.h>

//top bar
class TopBar : public juce::Component
{
public:
    TopBar();
    //setting new resized
    void resized() override;
    void paint(juce::Graphics&) override;

    // public
    juce::TextButton presetButton{ "Presets" };
    juce::TextButton bypassButton{ "Bypass" };
    juce::TextButton settingsButton{ "Settings" };

    //juce::Label pluginTitle{ {}, "Pitchblade" };
    juce::ImageComponent logo;
};