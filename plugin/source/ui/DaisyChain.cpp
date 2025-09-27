// reyna 

#include "Pitchblade/ui/DaisyChain.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "Pitchblade/ui/DaisyChainItem.h"
#include <algorithm>

DaisyChain::DaisyChain()
{
    rebuild();
}

static void reorderEffects(int fromIndex, int toIndex)
{
	//reorder in global effects list
    if (fromIndex < 0 || fromIndex >= (int)effects.size()) return;
    if (toIndex < 0 || toIndex >= (int)effects.size()) return;
    if (fromIndex == toIndex) return;

    auto item = effects[fromIndex];
    effects.erase(effects.begin() + fromIndex);
    effects.insert(effects.begin() + toIndex, item);
}

void DaisyChain::rebuild()
{
    // clear UI rows
    for (auto* it : items) removeChildComponent(it);
    items.clear();

    // create rows from the global effects list
    for (int i = 0; i < effects.size(); ++i) {
        auto* row = new DaisyChainItem(effects[i].name, i);
        // sync visuals to current effect bypass state
        row->updateBypassVisual(effects[i].bypassed);

        // hook callback to update global bypass state
        row->onBypassChanged = [i, row](int index, bool state) {
            effects[index].bypassed = state;
            row->updateBypassVisual(state); // keep visuals in sync immediately
            };

        // reorder callback
        row->onReorder = [this](int fromIndex, juce::String dragName, int targetIndex) {
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
        for (int i = 0; i < (int)effects.size(); ++i)
            if (effects[i].name == dragName) { resolvedFrom = i; break; }
    }
    if (resolvedFrom < 0) return; // safety

    // If dragging dragged, removing the source shifts the target left by 1
    if (resolvedFrom < targetIndex)
        targetIndex -= 1;

    if (targetIndex == resolvedFrom || targetIndex < 0 || targetIndex > (int)effects.size())
        return;

    // move the effect in the global vector
    auto item = effects[resolvedFrom];
    effects.erase(effects.begin() + resolvedFrom);
    effects.insert(effects.begin() + targetIndex, item);

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