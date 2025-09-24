//hudas code
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/effects/FormantDetector.h"

class FormantPanel : public juce::Component,
                     public juce::Button::Listener,     //To handle button clicks - huda
                     private juce::Timer                //adding a timer to update formants - huda
{
public:
    explicit FormantPanel(AudioPluginAudioProcessor& proc);
    //~FormantPanel() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;

    AudioPluginAudioProcessor& processor;


    bool showingFormants = false;

    juce::TextButton toggleViewButton{ "Show Formants" };
    juce::Slider gainSlider;

};