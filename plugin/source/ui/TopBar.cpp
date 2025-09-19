// reyna macabebe

#include "Pitchblade/ui/TopBar.h"

TopBar::TopBar()
{
    //makes ui  visible
    addAndMakeVisible(pluginTitle);
    addAndMakeVisible(settingsButton);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(presetButton);
    
}

void TopBar::resized()
{
    // laying out each component
    auto area = getLocalBounds();
    pluginTitle.setBounds(area.removeFromLeft(150));
    settingsButton.setBounds(area.removeFromRight(80));
    bypassButton.setBounds(area.removeFromRight(80));
    presetButton.setBounds(area.removeFromRight(80));
    
}