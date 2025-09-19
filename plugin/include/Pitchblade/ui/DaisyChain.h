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

    //  all effect buttons 
    juce::OwnedArray<juce::TextButton> effectButtons;
};