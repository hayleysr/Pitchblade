// austins code
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"

//gain panel ui
// creates slider n attaches to apvts parameter (in pluginprocessor.cpp)
class GainPanel : public juce::Component, public juce::ValueTree::Listener {
public:
    explicit GainPanel(AudioPluginAudioProcessor& proc);

	// overloaded constructor with local state
    GainPanel(AudioPluginAudioProcessor& proc, juce::ValueTree& state);

	// destructor
    ~GainPanel() override;

	// valueTree listener callback
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

    void resized() override;
    void paint(juce::Graphics&) override; 

private:
    //declaring slider n processor
    AudioPluginAudioProcessor& processor;
    //ui
    juce::Slider gainSlider;
    juce::Label gainLabel;

	// attachment to link slider to the apvts parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
	juce::ValueTree localState; // local state for this panel

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainPanel)
};

////////////////////////////////////////////////////////////
// reyna changes > added gain visualizer placeholder
// visualizer node for gain
#include "Pitchblade/ui/VisualizerPanel.h"

class GainVisualizer : public juce::Component,
    private juce::Timer
{
public:
    explicit GainVisualizer(AudioPluginAudioProcessor& proc) : processor(proc)
    {
        startTimerHz(30); // refresh ~30fps
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::pink);

        // get gain values
        float gainValue = processor.apvts.getRawParameterValue("GAIN")->load();
        
        // draw label
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        g.drawText("Gain visualizer placeholder", getLocalBounds().reduced(5), juce::Justification::centred);
    }

private:
    void timerCallback() override { repaint(); }

    AudioPluginAudioProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainVisualizer)
};


////////////////////////////////////////////////////////////

//reynas changes > added dsp node defn to ui panel creation
// gain dsp node , processes audio + makes own panel
// inherits from EffectNode base class
#include "Pitchblade/panels/EffectNode.h"
#include "Pitchblade/effects/GainProcessor.h"

class GainNode : public EffectNode {
public:
    explicit GainNode(AudioPluginAudioProcessor& proc) : EffectNode(proc, "GainNode", "Gain"), processor(proc) {   
        // initialize default properties
        if (!getMutableNodeState().hasProperty("Gain"))
            getMutableNodeState().setProperty("Gain", 0.0f, nullptr);
		// ensure EffectNodes tree exists
        if (!processor.apvts.state.hasType("EffectNodes"))
            processor.apvts.state = juce::ValueTree("EffectNodes");
		// add this node to processor state tree
        processor.apvts.state.addChild(getMutableNodeState(), -1, nullptr);
    }

	// dsp read from local state instead of apvts
    void process(AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override {
        juce::ignoreUnused(proc);
        const float gainDb = (float)getNodeState().getProperty("Gain", 0.0f);
        gainDSP.setGain(gainDb);
        gainDSP.process(buffer);
    }

    // return UI panel linked to node
    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override {
		return std::make_unique<GainPanel>(proc, getMutableNodeState()); // pass local state
    }

    // return visualizer 
    std::unique_ptr<juce::Component> createVisualizer(AudioPluginAudioProcessor& proc) override {
        return std::make_unique<GainVisualizer>(proc);
    }

    std::shared_ptr<EffectNode> clone() const override
	{   
        auto copiedTree = getNodeState().createCopy();                      // Copy ValueTree state
		copiedTree.setProperty("uuid", juce::Uuid().toString(), nullptr);   // new uuid for clone

		auto* self = const_cast<GainNode*>(this);                                           // to access processor ref
		auto clonePtr = std::make_shared<GainNode>(self->processor);                        // create new GainNode
		clonePtr->getMutableNodeState().copyPropertiesAndChildrenFrom(copiedTree, nullptr); // copy state

        // Keep clone in processor state tree
        self->processor.apvts.state.addChild(clonePtr->getMutableNodeState(), -1, nullptr);
        clonePtr->setDisplayName(effectName); // name will be made unique in daisychian
        return clonePtr;
    }

private:
	//nodes own dsp processor + reference to main processor for param access
    AudioPluginAudioProcessor& processor;
    GainProcessor gainDSP;
};