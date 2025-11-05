// austin
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"

//gain panel ui
class NoiseGatePanel : public juce::Component, public juce::ValueTree::Listener
{
public:
    explicit NoiseGatePanel(AudioPluginAudioProcessor& proc);
    void resized() override;
    void paint(juce::Graphics&) override;

    // overloaded constructor with local state
    NoiseGatePanel(AudioPluginAudioProcessor& proc, juce::ValueTree& state);

    // destructor
    ~NoiseGatePanel() override;

    // valueTree listener callback
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

private:
    // Reference back to main processor
    AudioPluginAudioProcessor& processor;

    // Sliders for noise gate
    juce::Slider thresholdSlider, attackSlider, releaseSlider;

    // Labels
    juce::Label noiseGateLabel, thresholdLabel, attackLabel, releaseLabel;

	// attachments to link sliders to the apvts parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    juce::ValueTree localState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseGatePanel)
};
////////////////////////////////////////////////////////////
// reyna changes > added noisegate visualizer placeholder
// visualizer node for noisegate
#include "Pitchblade/ui/VisualizerPanel.h"

class NoiseGateVisualizer : public juce::Component,
    private juce::Timer
{
public:
    explicit NoiseGateVisualizer(AudioPluginAudioProcessor& proc) : processor(proc)
    {
        startTimerHz(30); // refresh ~30fps
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::pink);

        // get noisegate values
        float thresholdValue = processor.apvts.getRawParameterValue("GATE_THRESHOLD")->load();
        float attackValue = processor.apvts.getRawParameterValue("GATE_ATTACK")->load();
        float releaseValue = processor.apvts.getRawParameterValue("GATE_RELEASE")->load();

        // draw label
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        g.drawText("Noisegate visualizer placeholder", getLocalBounds().reduced(5), juce::Justification::centred);
    }

private:
    void timerCallback() override { repaint(); }

    AudioPluginAudioProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseGateVisualizer)
};

////////////////////////////////////////////////////////////

//reynas changes > added dsp node defn to ui panel creation
// noisegate dsp node , processes audio + makes own panel
// inherits from EffectNode base class
#include "Pitchblade/panels/EffectNode.h"
#include "Pitchblade/effects/NoiseGateProcessor.h"

class NoiseGateNode : public EffectNode {
public:
	//create node with name n reference to main processor
    explicit NoiseGateNode(AudioPluginAudioProcessor& proc) : EffectNode(proc, "NoiseGateNode", "Noise Gate"), processor(proc) {
        // initialize default properties
        if (!getMutableNodeState().hasProperty("GateThreshold"))
            getMutableNodeState().setProperty("GateThreshold", -100.0f, nullptr);
        if (!getMutableNodeState().hasProperty("GateAttack"))
            getMutableNodeState().setProperty("GateAttack", 25.0f, nullptr);
        if (!getMutableNodeState().hasProperty("GateRelease"))
            getMutableNodeState().setProperty("GateRelease", 100.0f, nullptr);
        // ensure EffectNodes tree exists
        if (!processor.apvts.state.hasType("EffectNodes"))
            processor.apvts.state = juce::ValueTree("EffectNodes");
        // add this node to processor state tree
        processor.apvts.state.addChild(getMutableNodeState(), -1, nullptr);

        //Preparing the gate
        gateDSP.prepare(proc.getSampleRate());
    }

	// dsp processing step for noise gate
    void process(AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override {

        juce::ignoreUnused(proc);

        const float threshold = (float)getNodeState().getProperty("GateThreshold", -100.0f);
        const float attack = (float)getNodeState().getProperty("GateAttack", 25.0f);
        const float release = (float)getNodeState().getProperty("GateRelease", 100.0f);

        gateDSP.setThreshold(threshold);
        gateDSP.setAttack(attack);
        gateDSP.setRelease(release);
        gateDSP.process(buffer);
    }

    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override {
        return std::make_unique<NoiseGatePanel>(proc, getMutableNodeState());
    }

    // return visualizer 
    std::unique_ptr<juce::Component> createVisualizer(AudioPluginAudioProcessor& proc) override {
        return std::make_unique<NoiseGateVisualizer>(proc);
    }

    ////////////////////////////////////////////////////////////

    // clone node
    std::shared_ptr<EffectNode> clone() const override {
        auto copiedTree = getNodeState().createCopy();                      // Copy ValueTree state
        copiedTree.setProperty("uuid", juce::Uuid().toString(), nullptr);   // new uuid for clone

        auto* self = const_cast<NoiseGateNode*>(this);                                           // to access processor ref
        auto clonePtr = std::make_shared<NoiseGateNode>(self->processor);                        // create new GainNode
        clonePtr->getMutableNodeState().copyPropertiesAndChildrenFrom(copiedTree, nullptr); // copy state

        // Keep clone in processor state tree
        self->processor.apvts.state.addChild(clonePtr->getMutableNodeState(), -1, nullptr);
        clonePtr->setDisplayName(effectName); // name will be made unique in daisychian
        return clonePtr;
    }

private:
    //nodes own dsp processor + reference to main processor for param access
    AudioPluginAudioProcessor& processor;
    NoiseGateProcessor gateDSP;
};