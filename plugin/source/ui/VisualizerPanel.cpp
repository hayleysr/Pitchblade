// reyna
#include <JuceHeader.h>
#include "Pitchblade/ui/VisualizerPanel.h"
#include "Pitchblade/PluginProcessor.h"

#include "Pitchblade/panels/EffectNode.h"

VisualizerPanel::VisualizerPanel(AudioPluginAudioProcessor& proc, std::vector<std::shared_ptr<EffectNode>>& nodes)
                                                                        : processor(proc), effectNodes(nodes) {
    tabs.setTabBarDepth(0);  // hide tab bar
    addAndMakeVisible(tabs);
    refreshTabs();
}

void VisualizerPanel::paint(juce::Graphics& g)
{
    //placeholder
    g.fillAll(juce::Colours::black);
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

	// each node creates its own visualizer 
    for (auto& node : effectNodes)
    {
        auto visualizer = node->createVisualizer(processor);
        if (visualizer)
            tabs.addTab(node->effectName, juce::Colours::black, visualizer.release(), true);
        else
        {
            // default placeholder , currently for formant and pitch 
            auto* placeholder = new juce::Label({}, node->effectName + " Visualizer");
            placeholder->setJustificationType(juce::Justification::centred);
            placeholder->setFont(juce::Font(18.0f));
            tabs.addTab(node->effectName, juce::Colours::black, placeholder, true);
        }
    }
}