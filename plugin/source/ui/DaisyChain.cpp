#include "Pitchblade/ui/DaisyChain.h"

DaisyChain::DaisyChain()
{
    //create buttons
    for (auto name : { "Pitch", "Formant", "Equalizer", "Compressor", "Gate/Gain" })
    {
        //makes new one for each effect
        auto* btn = new juce::TextButton(name);
        effectButtons.add(btn);
        addAndMakeVisible(btn);
    }
}

void DaisyChain::resized()
{
    //vertical
    auto area = getLocalBounds().reduced(10);
    int y = 0;
    for (auto* btn : effectButtons)
        btn->setBounds(0, y += 40, area.getWidth(), 30);
}