// reyna
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/panels/EffectNode.h"
#include "Pitchblade/PluginProcessor.h"

//panel that shows audio visuals
class VisualizerPanel : public juce::Component
{
public:
	VisualizerPanel(AudioPluginAudioProcessor& proc, std::vector<std::shared_ptr<EffectNode>>& nodes);   // pass processor + effect nodes for tabs

    void resized() override;
    void paint(juce::Graphics& g) override;

    void showVisualizer(int index);
	void clearVisualizer();
    void refreshTabs();

private:
	AudioPluginAudioProcessor& processor;               // reference to main processor so visulaizer can access effect data
	std::vector<std::shared_ptr<EffectNode>>& effectNodes;          // global effect nodes
    juce::TabbedComponent tabs{ juce::TabbedButtonBar::TabsAtTop };
};