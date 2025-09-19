#pragma once
#include <JuceHeader.h>

//sidebar
class DaisyChain : public juce::Component
{
public:
    DaisyChain();
    void resized() override;

    //  all effect buttons 
    juce::OwnedArray<juce::TextButton> effectButtons;
};