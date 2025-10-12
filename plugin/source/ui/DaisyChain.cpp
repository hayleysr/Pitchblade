// reyna 

#include "Pitchblade/ui/DaisyChain.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "Pitchblade/ui/DaisyChainItem.h"
#include <algorithm>

DaisyChain::DaisyChain(std::vector<std::shared_ptr<EffectNode>>& nodes) : effectNodes(nodes) {
	// copy effect names from nodes
    effectNames.clear();
    for (auto& n : effectNodes)
        effectNames.push_back(n->effectName);

    rebuild();
}
// helper to find node by name
std::shared_ptr<EffectNode> DaisyChain::findNodeByName(const juce::String& name) const {

    for (auto& n : effectNodes)
        if (n->effectName == name)
            return n;
    return {};
}

void DaisyChain::rebuild()
{
    // clear UI rows
    for (auto* it : items) removeChildComponent(it);
    items.clear();

	// create rows from effectnodes // edited to use effectnodes // edited to use effectnames instead of directly using effectnodes
    for (int i = 0; i < (int)effectNames.size(); ++i) {
		const auto& name = effectNames[i];  // get name from current order list
		auto node = findNodeByName(name);   // find corresponding node

		auto* row = new DaisyChainItem(name, i);        // create row with effect name

        // insync visuals to current processor node bypass state
        const bool bypassState = node ? node->bypassed : false;
        row -> updateBypassVisual(bypassState);     

        // update global bypass state visually
        row -> onBypassChanged = [ this, name, row](int index, bool state) {
            effectNodes[index]->bypassed = state;
            row->updateBypassVisual(state);         // keep visuals insync
            };

		// hook chaining mode (dosnt work yet)
        row -> onModeChanged = [this, i]( int index, int modeId) {
            //effectNodes[index]->chainMode = modeId;
            };

        // reorder callback ui
        row -> onReorder = [this](int fromIndex, juce::String dragName, int targetIndex) {
                handleReorder(fromIndex, dragName, targetIndex);
            };
        addAndMakeVisible(row);
        items.add(row);
    }
    resized();
    repaint();
    if (globalBypassed)
        setGlobalBypassVisual(true);
}

//reorders the global effects list and rebuilds UI
void DaisyChain::handleReorder(int fromIndex, const juce::String& dragName, int targetIndex)
{
    int resolvedFrom = fromIndex;
    if (resolvedFrom < 0)
    {
        for (int i = 0; i < (int)effectNodes.size(); ++i)
            if (effectNodes[i] -> effectName == dragName) { resolvedFrom = i; break; }
    }
    if (resolvedFrom < 0) return; // safety

    // If dragging dragged, removing the source shifts the target left by 1
    if (resolvedFrom < targetIndex)
        targetIndex -= 1;

    if (targetIndex == resolvedFrom || targetIndex < 0 || targetIndex > (int)effectNodes.size())
        return;

    // move effect order ui only
    const auto movedName = effectNames[resolvedFrom];       //find name
	effectNames.erase(effectNames.begin() + resolvedFrom);  //remove from old pos
	effectNames.insert(effectNames.begin() + targetIndex, movedName);   //insert at new pos

    // Rebuild Ui
    rebuild();
	//reorder effect nodes to match new order at end
    if (onReorderFinished) onReorderFinished();
}

void DaisyChain::resized()
{
    auto area = getLocalBounds().reduced(4);
    area.removeFromTop(10);
    area.removeFromRight(8);

    for (auto* item : items)
        item->setBounds(area.removeFromTop(48));
}

void DaisyChain::paint(juce::Graphics& g)
{
    g.fillAll(Colors::panel);
    g.drawRect(getLocalBounds(), 2);
}

void DaisyChain::setGlobalBypassVisual(bool state)
{
    globalBypassed = state;
    for (auto* row : items)
    {
        if (globalBypassed)
        {
            // grey out
            row->bypass.setEnabled(false);
            row->bypass.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
            row->bypass.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkgrey);
            row->bypass.setColour(juce::TextButton::textColourOffId, Colors::background);
            row->bypass.setColour(juce::TextButton::textColourOnId, Colors::background);
        }
        else
        {
            // restore
            const bool state = row->bypassed;
            const auto bg = state ? juce::Colours::hotpink : Colors::panel;

            row->bypass.setEnabled(true);
            row->bypass.setColour(juce::TextButton::buttonColourId, bg);
            row->bypass.setColour(juce::TextButton::buttonOnColourId, juce::Colours::lightgrey);
            row->bypass.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            row->bypass.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        }
        row->bypass.repaint();
    }
}