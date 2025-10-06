// reyna macabebe
#include <JuceHeader.h>
#include "Pitchblade/ui/VisualizerPanel.h"
#include "Pitchblade/PluginProcessor.h"

#include "Pitchblade/panels/EffectNode.h"

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

    auto* placeholder = new juce::Label({}, "Visualizer");
    placeholder->setJustificationType(juce::Justification::centred);
    placeholder->setFont(juce::Font(18.0f, juce::Font::bold));

    tabs.addTab("Visualizer", juce::Colours::black, placeholder, true);
}