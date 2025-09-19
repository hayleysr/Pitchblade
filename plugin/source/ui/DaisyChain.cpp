// reyna macabebe

#include "Pitchblade/ui/DaisyChain.h"

DaisyChain::DaisyChain()
{
    //creates new buttons based off of effects list in effectpanel.cpp
    for (auto& e : effects)
    {
        auto* btn = new juce::TextButton(e.name);
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