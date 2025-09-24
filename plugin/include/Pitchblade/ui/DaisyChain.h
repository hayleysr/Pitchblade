// reyna macabebe

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/ui/EffectRegistry.h"


//sidebar
class DaisyChain : public juce::Component
{
public:
    DaisyChain();
    void resized() override;

    bool isBypassed();
    void setBypassed(bool newState);
    void paint(juce::Graphics&) override;

    //  all effect buttons 
    juce::OwnedArray<juce::TextButton> effectButtons;
    //bypass buttons
    juce::OwnedArray<juce::ToggleButton> bypassButtons;
};