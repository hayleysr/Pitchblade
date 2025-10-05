// reyna
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/panels/EffectNode.h"

//tabs for each effect
class EffectPanel : public juce::Component
{
public:
    explicit EffectPanel(AudioPluginAudioProcessor& proc, std::vector<std::shared_ptr<EffectNode>>& nodes);

    void resized() override;
    void showEffect(int index);
    void paint(juce::Graphics&) override;

	//refresh tabs when effects are added/removed
    void refreshTabs();

private:
    // tab buttons; side + top tabs
    AudioPluginAudioProcessor& processor;
    juce::TabbedComponent tabs{ juce::TabbedButtonBar::TabsAtTop };

    std::vector<std::shared_ptr<EffectNode>>& effectNodes;
};