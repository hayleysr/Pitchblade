// reyna

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/ui/DaisyChainItem.h"
#include "Pitchblade/panels/EffectNode.h"

//updated to work with effect nodes instead of vector
//sidebar
class DaisyChain : public juce::Component
{
public:
    DaisyChain(AudioPluginAudioProcessor& proc, std::vector<std::shared_ptr<EffectNode>>& nodes);

    void resized() override;
    void paint(juce::Graphics&) override;
	void setGlobalBypassVisual(bool globalBypassed);    // grayed out when global bypassed
    bool globalBypassed = false;

	//refeshes ui from effectnodes
    void rebuild();
    std::function<void()> onReorderFinished;
	
    std::vector<juce::String> getCurrentOrder() const { return effectNames; } //get current order of effects
    juce::OwnedArray<DaisyChainItem> items;

	//add + copy buttons
    juce::TextButton addButton{ "Add" };
    juce::TextButton duplicateButton{ "Copy" };
    juce::TextButton deleteButton{ "Del" };

    void showAddMenu();
    void showDuplicateMenu();
    void showDeleteMenu();

private:
    void handleReorder(int fromIndex, const juce::String& targetName, int targetIndex);
	std::shared_ptr<EffectNode> findNodeByName(const juce::String& name) const; // helper to find node by name
    std::vector<std::shared_ptr<EffectNode>>& effectNodes;  // refern to processor's chain

	std::vector<juce::String> effectNames; // current order of effect names

    AudioPluginAudioProcessor& processorRef; // store processor reference
};