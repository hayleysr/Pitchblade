// reyna 

#include "Pitchblade/ui/DaisyChain.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "Pitchblade/ui/DaisyChainItem.h"
#include <algorithm>

DaisyChain::DaisyChain(std::vector<std::shared_ptr<EffectNode>>& nodes)
    : effectNodes(nodes)
{
    rebuild();
}

void DaisyChain::rebuild()
{
    // clear UI rows
    for (auto* it : items) removeChildComponent(it);
    items.clear();

	// create rows from effectnodes // edited to use effectnodes
    for (int i = 0; i < effectNodes.size(); ++i) {
        auto* row = new DaisyChainItem(effectNodes[i] -> effectName, i);
        // sync visuals to current effect bypass state
        row -> updateBypassVisual(effectNodes[i]->bypassed);

        // hook callback to update global bypass state
        row -> onBypassChanged = [ this, i, row](int index, bool state) {
            effectNodes[index]->bypassed = state;
            row->updateBypassVisual(state);         // keep visuals insync
            };

		// hook mode chain change callback
        row -> onModeChanged = [this, i]( int index, int modeId) {
            effectNodes[index]->chainMode = modeId;
            };

        // reorder callback
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

    // move the node
    auto node = effectNodes[resolvedFrom];
    effectNodes.erase(effectNodes.begin() + resolvedFrom);
    effectNodes.insert(effectNodes.begin() + targetIndex, node);

    // reconnect linearly after reorder
    for (int i = 0; i + 1 < effectNodes.size(); ++i)
        effectNodes[i]->clearConnections(), effectNodes[i]->connectTo(effectNodes[i + 1]);

    // Rebuild UI and notify
    rebuild();
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