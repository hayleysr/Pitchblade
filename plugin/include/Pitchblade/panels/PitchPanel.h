/**
 * Author: Hayley Spellicy-Ryan
 */
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h" 
#include "Pitchblade/effects/PitchDetector.h"

class PitchPanel : public juce::Component,
                   private juce::Timer          // Update UI at regular intervals
{
public:
    explicit PitchPanel(AudioPluginAudioProcessor& proc);

    void resized() override;
    void paint(juce::Graphics& g) override;
private:
    void timerCallback() override;

    AudioPluginAudioProcessor& processor;
};