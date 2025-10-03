// reyna macabebe

#include "Pitchblade/ui/VisualizerPanel.h"
#include "Pitchblade/ui/EffectRegistry.h"
#include "Pitchblade/PluginProcessor.h"

VisualizerPanel::VisualizerPanel() {
    tabs.setTabBarDepth(0);  // hide tab bar
    addAndMakeVisible(tabs);
}

void VisualizerPanel::paint(juce::Graphics& g)
{
    //placeholder
    g.fillAll(juce::Colours::black);
    /*g.setColour(juce::Colours::white);
    g.drawText("Visualizer Placeholder", getLocalBounds(), juce::Justification::centred);*/
}

void VisualizerPanel::resized()
{
    tabs.setBounds(getLocalBounds());
}

void VisualizerPanel::showVisualizer(int index)
{
    tabs.setCurrentTabIndex(index);
}

void VisualizerPanel::refreshTabs()
{
    tabs.clearTabs();

    for (auto& e : effects)
    {
        auto* placeholder = new juce::Label({}, "Visualizer for " + e.name);
        placeholder->setJustificationType(juce::Justification::centred);
        tabs.addTab(e.name, juce::Colours::black, placeholder, true);
    }
}