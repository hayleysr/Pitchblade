// reyna 
// replacing effectRegistry.h's vectors with node based decorator pattern

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include <memory>
#include <vector>

class AudioPluginAudioProcessor;

//base class for all effects in daisychain
// nodes defined in each individual effectPanel.h 
class EffectNode : public juce::Component, private juce::ValueTree::Listener {
public:
	// constructor for new node
    EffectNode(AudioPluginAudioProcessor& proc, const juce::String& type, const juce::String& displayName) : processor(proc),
                                                                            nodeType(type), effectName(displayName), nodeState(juce::ValueTree(juce::Identifier(type))) {
		nodeState.setProperty("name", effectName, nullptr);                 // set display name
		nodeState.setProperty("uuid", juce::Uuid().toString(), nullptr);    // unique id
		nodeState.addListener(this);                                        // listen to state changes
    }

	// constructor from existing state
    EffectNode(AudioPluginAudioProcessor& proc, const juce::ValueTree& existingState) : processor(proc),
                                                                        nodeType(existingState.getType().toString()),
                                                                        effectName(existingState.getProperty("name", "Effect").toString()), nodeState(existingState) {
		jassert(nodeState.isValid());   // ensure valid state
		nodeState.addListener(this);  
    }

    ~EffectNode() override { nodeState.removeListener(this); }                                          // destructor

    virtual void process(AudioPluginAudioProcessor& proc,juce::AudioBuffer<float>& buffer) = 0;         // process the incoming buffer 

     virtual std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) = 0;         // ui panel creation 

	 virtual std::unique_ptr<juce::Component> createVisualizer(AudioPluginAudioProcessor& proc) {       //visualizer creation
         juce::ignoreUnused(proc);
         return nullptr; 
     }

	 virtual std::shared_ptr<EffectNode> clone() const = 0;      // duplicate node

	 // processing through the node and its outputs for different chain modes
    void processAndForward(AudioPluginAudioProcessor& proc,
        juce::AudioBuffer<float>& buffer)
    {
        if (!bypassed)
            process(proc, buffer);
        if (!outputs.empty())
            outputs[0]->processAndForward(proc, buffer);
    }

    // Connect this node to another 
	// stores next node into thee outputs vector
    void connectTo(std::shared_ptr<EffectNode> next) { 
		if (next.get() == this) return;     // Prevent self-connection w copys
        outputs.clear();
        outputs.push_back(next); 
    }

    // Clear all downstream connections 
    void clearConnections() {
        outputs.clear(); 
    }

    /////////////////////////////
    
    // Accessors
	const juce::String& getNodeType()  const { return nodeType; }           // type of node
	const juce::ValueTree& getNodeState() const { return nodeState; }       // state of node
	juce::ValueTree& getMutableNodeState() { return nodeState; }            //  mutable state of node

	const juce::ValueTree& getNodeStateConst() const { return nodeState; }  // const state of node
	juce::ValueTree& getNodeStateRef() { return nodeState; }                // reference to state of node
	const juce::String& getNodeTypeConst() const { return nodeType; }       // const type of node


    void setDisplayName(const juce::String& newName) {
        effectName = newName;
        nodeState.setProperty("name", effectName, nullptr);
    }

    juce::String effectName;
    bool bypassed = false;
    //int chainMode = 1; // 1 = down, 2 = split, 3 = double, 4 = unite
    
/////////////////////////////
protected:
    std::vector<std::shared_ptr<EffectNode>> outputs; 

	AudioPluginAudioProcessor& processor;   // reference to main processor
	juce::String nodeType;                  // type of effect node
	juce::ValueTree nodeState;              // state of effect node

	// valuetree listener callback
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override {
        juce::ignoreUnused(tree, property);
    }
};

