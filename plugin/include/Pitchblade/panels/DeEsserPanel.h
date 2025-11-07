// Written by Austin Hills
// Refactored to use local ValueTree state

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/panels/EffectNode.h"
#include "Pitchblade/effects/DeEsserProcessor.h"

class DeEsserPanel : public juce::Component, public juce::ValueTree::Listener
{
public:
    explicit DeEsserPanel(AudioPluginAudioProcessor& proc, juce::ValueTree& state);
    ~DeEsserPanel() override;

    void resized() override;
    void paint(juce::Graphics&) override;

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

private:
    //Reference to main processor
    AudioPluginAudioProcessor& processor;

    //Sliders
    juce::Slider thresholdSlider, ratioSlider, attackSlider, releaseSlider, frequencySlider;

    //Labels
    juce::Label deEsserLabel, thresholdLabel, ratioLabel, attackLabel, releaseLabel, frequencyLabel;

    //Attachments to link stuff to APVTS parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> frequencyAttachment;

    juce::ValueTree localState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeEsserPanel)
};

//Visualizer Node for DeEsser
#include "Pitchblade/ui/VisualizerPanel.h"

class DeEsserVisualizer : public juce::Component,
    private juce::Timer
{
public:
    explicit DeEsserVisualizer(AudioPluginAudioProcessor& proc) : processor(proc)
    {
        startTimerHz(30); // refresh ~30fps
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::pink);

        // get de-esser values
        float thresholdValue = processor.apvts.getRawParameterValue("DEESSER_THRESHOLD")->load();
        float ratioValue = processor.apvts.getRawParameterValue("DEESSER_RATIO")->load();
        float attackValue = processor.apvts.getRawParameterValue("DEESSER_ATTACK")->load();
        float releaseValue = processor.apvts.getRawParameterValue("DEESSER_RELEASE")->load();
        float frequencyValue = processor.apvts.getRawParameterValue("DEESSER_FREQUENCY")->load();

        // draw label
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        g.drawText("De-esser visualizer placeholder", getLocalBounds().reduced(5), juce::Justification::centred);
    }

private:
    void timerCallback() override { repaint(); }

    AudioPluginAudioProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeEsserVisualizer)
};

//Austin
class DeEsserNode : public EffectNode
{
public:
    //Create node with name and reference to main processor
    explicit DeEsserNode(AudioPluginAudioProcessor& proc) : EffectNode(proc, "DeEsserNode", "De-Esser"), processor(proc){
        //initialize default properties
        if (!getMutableNodeState().hasProperty("DeEsserThreshold"))
            getMutableNodeState().setProperty("DeEsserThreshold", 0.0f, nullptr);
        if (!getMutableNodeState().hasProperty("DeEsserRatio"))
            getMutableNodeState().setProperty("DeEsserRatio", 4.0f, nullptr);
        if (!getMutableNodeState().hasProperty("DeEsserAttack"))
            getMutableNodeState().setProperty("DeEsserAttack", 5.0f, nullptr);
        if (!getMutableNodeState().hasProperty("DeEsserRelease"))
            getMutableNodeState().setProperty("DeEsserRelease", 5.0f, nullptr);
        if (!getMutableNodeState().hasProperty("DeEsserFrequency"))
            getMutableNodeState().setProperty("DeEsserFrequency", 6000.0f, nullptr);
        
        //Ensure EffectNodes tree exists
        if (!processor.apvts.state.hasType("EffectNodes"))
            processor.apvts.state = juce::ValueTree("EffectNodes");

        //Add this node to processor state tree
        processor.apvts.state.addChild(getMutableNodeState(), -1, nullptr);

        //Preparing the de-esser
        deEsserDSP.prepare(proc.getSampleRate(),proc.getCurrentBlockSize());
    }

    //dsp processing step for de-esser
    void process(AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override{
        juce::ignoreUnused(proc);

        const float threshold = (float)getNodeState().getProperty("DeEsserThreshold", 0.0f);
        const float ratio = (float)getNodeState().getProperty("DeEsserRatio", 4.0f);
        const float attack = (float)getNodeState().getProperty("DeEsserAttack", 5.0f);
        const float release = (float)getNodeState().getProperty("DeEsserRelease", 5.0f);
        const float frequency = (float)getNodeState().getProperty("DeEsserFrequency", 6000.0f);

        deEsserDSP.setThreshold(threshold);
        deEsserDSP.setRatio(ratio);
        deEsserDSP.setAttack(attack);
        deEsserDSP.setRelease(release);
        deEsserDSP.setFrequency(frequency);

        deEsserDSP.process(buffer);
    }

    //return UI panel linked to node
    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override
    {
        return std::make_unique<DeEsserPanel>(proc, getMutableNodeState());
    }

    std::unique_ptr<juce::Component> createVisualizer(AudioPluginAudioProcessor& proc) override{
        return std::make_unique<DeEsserVisualizer>(proc);
    }

    ////////////////////////////////////////////////////////  reyna

    //clone node
    std::shared_ptr<EffectNode> clone() const override
    {
        auto copiedTree = getNodeState().createCopy();
        copiedTree.setProperty("uuid", juce::Uuid().toString(), nullptr);

        auto* self = const_cast<DeEsserNode*>(this);
        auto clonePtr = std::make_shared<DeEsserNode>(self->processor);
        clonePtr->getMutableNodeState().copyPropertiesAndChildrenFrom(copiedTree, nullptr);

        self->processor.apvts.state.addChild(clonePtr->getMutableNodeState(), -1, nullptr);
        clonePtr->setDisplayName(effectName);
        return clonePtr;
    }

    // XML serialization for saving/loading
    std::unique_ptr<juce::XmlElement> toXml() const override;
    void loadFromXml(const juce::XmlElement& xml) override;

private:
    //nodes own dsp processor + reference to main processor for param access
    AudioPluginAudioProcessor& processor;
    DeEsserProcessor deEsserDSP;
};