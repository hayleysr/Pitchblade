// reyna

#include "Pitchblade/ui/EffectPanel.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"

#include "Pitchblade/panels/EffectNode.h"

//effects panel section
EffectPanel::EffectPanel(AudioPluginAudioProcessor& proc, const std::vector<std::shared_ptr<EffectNode>>& nodes)    //const read only reference to vector
    : processor(proc), effectNodes(nodes)
{
    refreshTabs();

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
    g.drawRect(getLocalBounds(), 2);
}

void EffectPanel::refreshTabs()
{
    //rebuilds tabs based on gobal effects list
    effectNodes = processor.getEffectNodes();
    tabs.clearTabs();
    for (auto& node : effectNodes) {
        if (!node) continue; // skip invalid       
		{   // create panel from node
            auto panel = node->createPanel(processor);
            if (panel)   
                tabs.addTab(node->effectName, Colors::background, panel.release(), true);
        }
    }
}