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
class EffectNode {
public:
    EffectNode(const juce::String& name) : effectName(name) {}
    virtual ~EffectNode() = default;

    // process the incoming buffer 
    virtual void process(AudioPluginAudioProcessor& proc,juce::AudioBuffer<float>& buffer) = 0;

	// ui panel creation 
     virtual std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) = 0;

    //visualizer creation
	 virtual std::unique_ptr<juce::Component> createVisualizer(AudioPluginAudioProcessor& proc) {   
         juce::ignoreUnused(proc);
         return nullptr; 
     }
	 // processing through the node and its outputs for different chain modes
    void processAndForward(AudioPluginAudioProcessor& proc,
        juce::AudioBuffer<float>& buffer)
    {
        if (!bypassed)
            process(proc, buffer);

        switch (chainMode)
        {
        case 1: 
            // Single down: send buffer to one output
            if (!outputs.empty())
                outputs[0]->processAndForward(proc, buffer);
            break;
        case 2: 
            // Split into double: duplicate buffer to each output
            if (!outputs.empty())
                outputs[0]->processAndForward(proc, buffer);
            break;

        case 3: 
            // Double down: expects two inputs merged 
            if (!outputs.empty())
                outputs[0]->processAndForward(proc, buffer);
            break;
        case 4: 
            // Unite into single: combine buffers 
            if (!outputs.empty())
                outputs[0]->processAndForward(proc, buffer);
            break;
        }
    }

    // Connect this node to another 
	// stores next node into thee outputs vector
    void connectTo(std::shared_ptr<EffectNode> next) { 
        outputs.push_back(next); 
    }

    // Clear all downstream connections 
    void clearConnections() {
        outputs.clear(); 
    }

    /////////////////////////////
    juce::String effectName;
    bool bypassed = false;
    int chainMode = 1; // 1 = down, 2 = split, 3 = double, 4 = unite

protected:
    std::vector<std::shared_ptr<EffectNode>> outputs; 
};

