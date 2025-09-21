// reyna macabebe

#include "Pitchblade/ui/EffectPanel.h"
#include "Pitchblade/ui/EffectRegistry.h"


//effects panel section
EffectPanel::EffectPanel(AudioPluginAudioProcessor& proc)
{
    for (auto& e : effects)
        tabs.addTab(e.name, juce::Colours::darkgrey, e.createPanel(proc), true);
    //hide top tab buttons
    tabs.setTabBarDepth(0);
    addAndMakeVisible(tabs);
}

void EffectPanel::resized()
{
    //get bounds of panel and sets
    tabs.setBounds(getLocalBounds());
}

void EffectPanel::showEffect(int index)
{
    //switches current tab
    tabs.setCurrentTabIndex(index);
}