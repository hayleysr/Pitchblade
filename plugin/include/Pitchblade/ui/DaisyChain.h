// reyna

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/ui/EffectRegistry.h"
#include "Pitchblade/ui/DaisyChainItem.h"


//sidebar
class DaisyChain : public juce::Component
{
public:
    DaisyChain();
    void resized() override;

    void paint(juce::Graphics&) override;
	void setGlobalBypassVisual(bool globalBypassed);    // grayed out when global bypassed
    bool globalBypassed = false;

	//refresh rows 
    void rebuild();
    std::function<void()> onReorderFinished;
    juce::OwnedArray<DaisyChainItem> items;

private:
    void handleReorder(int fromIndex, const juce::String& targetName, int targetIndex);
};