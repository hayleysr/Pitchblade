// reyna macabebe

#include "Pitchblade/ui/DaisyChain.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"

DaisyChain::DaisyChain()
{
    //creates new buttons based off of effects list in effectpanel.cpp
    for (int i = 0; i < effects.size(); ++i)
    {
        auto& e = effects[i];

        auto* btn = new juce::TextButton(e.name);
        effectButtons.add(btn);
        addAndMakeVisible(btn);

        auto* bypass = new juce::ToggleButton("");
        bypassButtons.add(bypass);
        addAndMakeVisible(bypass);

        // toggle bypass state for this effect
        bypass->onClick = [i, bypass]() {
            effects[i].bypassed = bypass->getToggleState();
            };
    }
}

void DaisyChain::resized()
{
    //vertical
    auto area = getLocalBounds().reduced(10);
    int y = 0;
    for (int i = 0; i < effectButtons.size(); ++i)
    {
        auto row = area.removeFromTop(40);  // one row per effect
        effectButtons[i]->setBounds(row.removeFromLeft(row.getWidth() - 25)); // left side
        bypassButtons[i]->setBounds(row);  // right side (80px wide)
    }
}

void DaisyChain::paint(juce::Graphics& g)
{
    g.fillAll(Colors::panel);
    g.drawRect(getLocalBounds(), 2);
}
