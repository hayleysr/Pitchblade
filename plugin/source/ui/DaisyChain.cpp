// reyna 

#include "Pitchblade/ui/DaisyChain.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "Pitchblade/ui/DaisyChainItem.h"
#include <algorithm>

// for the add/duplicate/delete menus
#include "Pitchblade/panels/GainPanel.h"
#include "Pitchblade/panels/NoiseGatePanel.h"
#include "Pitchblade/panels/CompressorPanel.h"
#include "Pitchblade/panels/FormantPanel.h"
#include "Pitchblade/panels/PitchPanel.h"

// helper to make unique effect names when adding/duplicating
static juce::String makeUniqueName(const juce::String& baseName, const std::vector<std::shared_ptr<EffectNode>>& nodes) {
    int counter = 1;
    juce::String newName = baseName;
	// lambda to check if name exists
    auto nameExists = [&](const juce::String& name) {
            for (auto& n : nodes)
				if (n && n->effectName == name)     // check against effect names in nodes
                    return true;
            return false;
        };
	
    while (nameExists(newName))
        newName = baseName + " " + juce::String(++counter); // ++ until unique

    return newName;
}

DaisyChain::DaisyChain(AudioPluginAudioProcessor& proc, std::vector<std::shared_ptr<EffectNode>>& nodes) :processorRef(proc), effectNodes(nodes) {
	// copy effect names from nodes
    effectNames.clear();
    for (auto& n : effectNodes)
        effectNames.push_back(n->effectName);

    rebuild();

	// add + duplicate buttons
    addAndMakeVisible(addButton);
    addAndMakeVisible(duplicateButton);
    addAndMakeVisible(deleteButton);

    addButton.onClick = [this]() { showAddMenu(); };
    duplicateButton.onClick = [this]() { showDuplicateMenu(); };
    deleteButton.onClick = [this]() { showDeleteMenu(); };
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
			auto node = findNodeByName(name);   // find corresponding node by name
            if (node)
                node->bypassed = state;
            row->updateBypassVisual(state);         // keep visuals insync
            };

		// chaining mode //////////////////////////////////
        row -> onModeChanged = [this]( int index, int modeId) {
			if (index < 0 || index >= items.size()) {   //safety check
                return;
            }

            // get the effect name from ui
            juce::String effectName = items[index]->getName();

            /// find the matching EffectNode by name
            auto& nodes = processorRef.getEffectNodes();
            for (auto& node : nodes) {
                if (node && node->effectName == effectName) {
					node->chainMode = static_cast<ChainMode>(modeId);   // update chain mode
                    break;
                }
            }
            processorRef.requestReorder(getCurrentOrder());  // rebuild processor chain

			// debug output: log updated chain modes
            juce::Logger::outputDebugString("Chain Mode Updated >>>");
            for (int j = 0; j < effectNodes.size(); ++j) {
                auto n = effectNodes[j];
                juce::String modeName;

                switch (n->chainMode) {
                case ChainMode::Down: modeName      = "Down"; break;
                case ChainMode::Split: modeName     = "Split"; break;
                case ChainMode::DoubleDown: modeName = "DoubleDown"; break;
                case ChainMode::Unite: modeName     = "Unite"; break;
                }
                juce::Logger::outputDebugString("[" + juce::String(j) + "] " + n->effectName + " -> Mode: " + modeName);
            }
            juce::Logger::outputDebugString("========================================");
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

	//add + duplicate buttons + delete
    auto bottomArea = area.removeFromTop(40);
    const int buttonWidth = 45;
    const int spacing = 5;

    auto rightEdge = getWidth() - 16;
    deleteButton.setBounds(rightEdge - buttonWidth, bottomArea.getY() + 4, buttonWidth, bottomArea.getHeight() - 8);
    duplicateButton.setBounds(deleteButton.getX() - buttonWidth - spacing, deleteButton.getY(), buttonWidth, deleteButton.getHeight());
    addButton.setBounds(duplicateButton.getX() - buttonWidth - spacing, duplicateButton.getY(), buttonWidth, duplicateButton.getHeight());
}

void DaisyChain::paint(juce::Graphics& g)
{
    g.fillAll(Colors::panel);
    g.drawRect(getLocalBounds(), 2);
}

// grey out individual bypass when global bypassed
void DaisyChain::setGlobalBypassVisual(bool state) {
    globalBypassed = state;
    for (auto* row : items) {
        if (globalBypassed) {
            // grey out
            row->bypass.setEnabled(false);
            row->bypass.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
            row->bypass.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkgrey);
            row->bypass.setColour(juce::TextButton::textColourOffId, Colors::background);
            row->bypass.setColour(juce::TextButton::textColourOnId, Colors::background);
        } else {
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
//////////////////////////////////// menus ///////////////////////////////////////////////////////////

// menu to add new effect nodes
void DaisyChain::showAddMenu() {
	juce::PopupMenu menu;       // create menu manually add all effects
    menu.addItem(1, "Gain");
    menu.addItem(2, "Noise Gate");
    menu.addItem(3, "Compressor");
    menu.addItem(4, "Formant");
    menu.addItem(5, "Pitch");

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&addButton), [this](int result) {
            if (result == 0) return;
			// create new node based on selection
            std::shared_ptr<EffectNode> newNode;
            switch (result) {
            case 1: newNode = std::make_shared<GainNode>(processorRef); break;
            case 2: newNode = std::make_shared<NoiseGateNode>(processorRef); break;
            case 3: newNode = std::make_shared<CompressorNode>(processorRef); break;
            case 4: newNode = std::make_shared<FormantNode>(processorRef); break;
            case 5: newNode = std::make_shared<PitchNode>(processorRef); break;
            }

			// add to processor + ui lists
            if (newNode) {
                newNode->effectName = makeUniqueName(newNode->effectName, effectNodes);
				// create and attach ValueTree to APVTS
                effectNodes.push_back(newNode);
                effectNames.push_back(newNode->effectName);

				// rebuild the chain
                auto oldCb = onReorderFinished;
                onReorderFinished = nullptr;
                rebuild();
                onReorderFinished = oldCb;
                
                if (onReorderFinished) onReorderFinished();
            }
        });
}

// menu to duplicate existing effect nodes
void DaisyChain::showDuplicateMenu() {
	juce::PopupMenu menu; // create menu with existing effect names
    for (int i = 0; i < effectNodes.size(); ++i) {
        menu.addItem(i + 1, effectNodes[i]->effectName);
    }
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&duplicateButton),
        [this](int result)
        {
            if (result == 0) return;
            const int index = result - 1;
            if (index < 0 || index >= effectNodes.size()) return;

            auto original = effectNodes[index];
            if (!original) return;

            // clone node object
            auto clone = original->clone();
            if (!clone) return;

            // clone its valueTree (parameters) and attach to apvts
            juce::ValueTree clonedTree(original->getNodeTypeConst() + "_" + juce::Uuid().toString());
            clonedTree.copyPropertiesAndChildrenFrom(original->getNodeStateConst(), nullptr);

			processorRef.apvts.state.addChild(clonedTree, -1, nullptr);     // add to apvts
			clone->getNodeStateRef() = clonedTree;          // set cloned tree to new node

			clone->effectName = makeUniqueName(original->effectName, effectNodes);  // make unique name
			effectNodes.push_back(clone);                   // add to processor list
			effectNames.push_back(clone->effectName);       // add to ui order list

			auto oldCb = onReorderFinished;     // rebuild chain
            onReorderFinished = nullptr;    
            rebuild();
            onReorderFinished = oldCb;          
            if (onReorderFinished) onReorderFinished();
        });
}
// menu to delete existing effect nodes
void DaisyChain::showDeleteMenu() {
	juce::PopupMenu menu;   // create menu with existing effect names
    for (int i = 0; i < effectNodes.size(); ++i) {
        menu.addItem(i + 1, effectNodes[i]->effectName);
    }

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&deleteButton),
        [this](int result)
        {
            if (result == 0) return; // user canceled
            const int index = result - 1;
            if (index < 0 || index >= effectNodes.size()) return;

            // remove from processor + UI lists
            effectNodes.erase(effectNodes.begin() + index);
            effectNames.erase(effectNames.begin() + index);

            // rebuild the chain
            rebuild();
            if (onReorderFinished)
                onReorderFinished();
        });
}

