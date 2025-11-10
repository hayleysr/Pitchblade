// reyna

#include "Pitchblade/ui/EffectPanel.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "BinaryData.h"
#include "Pitchblade/panels/EffectNode.h"

//effects panel section
EffectPanel::EffectPanel(AudioPluginAudioProcessor& proc, const std::vector<std::shared_ptr<EffectNode>>& nodes)    //const read only reference to vector
    : processor(proc), effectNodes(nodes)
{
    refreshTabs();

    //hide top tab buttons
    tabs.setTabBarDepth(0);
    tabs.setOpaque(false);
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
    juce::Image bg = juce::ImageCache::getFromMemory(
        BinaryData::panel_bg_png, BinaryData::panel_bg_pngSize);

    g.setColour(Colors::background.withAlpha(0.8f));

    if (bg.isValid()) {
        g.drawImage(bg, getLocalBounds().toFloat());
    }
    else
        g.fillAll(Colors::background);

    g.fillRect(getLocalBounds());
    g.drawRect(getLocalBounds(), 2);
}

void EffectPanel::refreshTabs()
{
    std::lock_guard<std::recursive_mutex> lock(processor.getMutex());
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