// reyna

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
    void setButtonActive(juce::TextButton& button, bool active);    // for coloring button

    // public
    juce::TextButton presetButton{ "Presets" };
    juce::TextButton bypassButton{ "Bypass" };
    juce::TextButton settingsButton{ "Settings" };

    juce::ImageComponent logo;
};