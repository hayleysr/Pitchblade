// reyna macabebe

#include "Pitchblade/ui/EffectPanel.h"
#include "Pitchblade/ui/EffectRegistry.h"
#include "Pitchblade/effects/GainProcessor.h"
#include "Pitchblade/panels/GainPanel.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"


//effects panel section
EffectPanel::EffectPanel(AudioPluginAudioProcessor& proc)
{
    for (auto& e : effects)
        tabs.addTab(e.name, Colors::background, e.createPanel(proc), true);
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


void EffectPanel::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);
    //g.setColour(Colors::accent);
    g.drawRect(getLocalBounds(), 2);
}