// Written by Austin Hills

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"

//Defines the UI panel
class CompressorPanel : public juce::Component, public juce::ValueTree::Listener
{
public:
    explicit CompressorPanel(AudioPluginAudioProcessor& proc, juce::ValueTree& state);
    ~CompressorPanel() override;

    void resized() override;
    void paint(juce::Graphics&) override;

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

private:
    // Reference back to main processor
    AudioPluginAudioProcessor& processor;

    //Sliders
    juce::Slider thresholdSlider, ratioSlider, attackSlider, releaseSlider;

    //Button for mode switching
    juce::TextButton modeButton {"Limiter Mode"};

    //Labels for sliders
    juce::Label compressorLabel, thresholdLabel, ratioLabel, attackLabel, releaseLabel;

    //Attachments to link stuff to APVTS parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> modeAttachment;

    juce::ValueTree localState;

    //Update visibility function
    void updateSliderVisibility();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorPanel)
};

// Creating visualizer node for compressor
#include "Pitchblade/ui/VisualizerPanel.h"

class CompressorVisualizer : public juce::Component, private juce::Timer
{
public:
    explicit CompressorVisualizer(AudioPluginAudioProcessor& proc) : processor(proc){
        startTimerHz(30); //refresh 30fps
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::aqua);

        // get compressor values
        float thresholdValue = processor.apvts.getRawParameterValue("COMP_THRESHOLD")->load();
        float ratioValue = processor.apvts.getRawParameterValue("COMP_RATIO")->load();
        float attackValue = processor.apvts.getRawParameterValue("COMP_ATTACK")->load();
        float releaseValue = processor.apvts.getRawParameterValue("COMP_RELEASE")->load();
        float limiter = processor.apvts.getRawParameterValue("COMP_LIMITER_MODE")->load();
        
        // draw label
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        g.drawText("Compressor visualizer placeholder", getLocalBounds().reduced(5), juce::Justification::centred);
    }
private:
    void timerCallback() override { repaint(); }

    AudioPluginAudioProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorVisualizer)
};

//Austin (copying reyna's formatting)
#include "Pitchblade/panels/EffectNode.h"
#include "Pitchblade/effects/CompressorProcessor.h"

class CompressorNode : public EffectNode 
{
public:
    //Create node with name and reference to main processor
    explicit CompressorNode(AudioPluginAudioProcessor& proc) : EffectNode(proc, "CompressorNode", "Compressor"), processor(proc) { 
        // initialize default properties
        if (!getMutableNodeState().hasProperty("CompThreshold"))
            getMutableNodeState().setProperty("CompThreshold", 0.0f, nullptr);
        if (!getMutableNodeState().hasProperty("CompRatio"))
            getMutableNodeState().setProperty("CompRatio", 3.0f, nullptr);
        if (!getMutableNodeState().hasProperty("CompAttack"))
            getMutableNodeState().setProperty("CompAttack", 50.0f, nullptr);
        if (!getMutableNodeState().hasProperty("CompRelease"))
            getMutableNodeState().setProperty("CompRelease", 250.0f, nullptr);
        if (!getMutableNodeState().hasProperty("CompLimiterMode"))
            getMutableNodeState().setProperty("CompLimiterMode", false, nullptr);

        //ensure EffectNodes tree exists
        if (!processor.apvts.state.hasType("EffectNodes"))
            processor.apvts.state = juce::ValueTree("EffectNodes");

        //add this node to processor state tree
        processor.apvts.state.addChild(getMutableNodeState(), -1, nullptr);
    }

    // dsp processing step for compressor
    void process(AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override {

        juce::ignoreUnused(proc);

        const float threshold = (float)getNodeState().getProperty("CompThreshold", 0.0f);
        const float ratio = (float)getNodeState().getProperty("CompRatio", 3.0f);
        const float attack = (float)getNodeState().getProperty("CompAttack", 50.0f);
        const float release = (float)getNodeState().getProperty("CompRelease", 250.0f);
        const bool isLimiterMode = (bool)getNodeState().getProperty("CompLimiterMode", false);
        
        //Check if the limiter mode is active
        if (isLimiterMode)
        {
            // In limiter mode, use a high fixed ratio and fast attack
            compressorDSP.setThreshold(threshold);
            compressorDSP.setRatio(2000.0f); // High, fixed ratio
            compressorDSP.setAttack(1.0f);   // Very fast attack
            compressorDSP.setRelease(release);
        }
        else
        {
            // In simple compressor mode, use the values from the sliders
            compressorDSP.setThreshold(threshold);
            compressorDSP.setRatio(ratio);
            compressorDSP.setAttack(attack);
            compressorDSP.setRelease(release);
        }

        // Process the audio buffer with the updated settings
        compressorDSP.process(buffer);
    }

    // return UI panel linked to node
    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override {
        return std::make_unique<CompressorPanel>(proc, getMutableNodeState());
    }

    // return visualizer 
    std::unique_ptr<juce::Component> createVisualizer(AudioPluginAudioProcessor& proc) override {
        return std::make_unique<CompressorVisualizer>(proc);
    }

    ////////////////////////////////////////////////////////////

    // clone node
    std::shared_ptr<EffectNode> clone() const override {
        auto copiedTree = getNodeState().createCopy();                    // Copy ValueTree state
        copiedTree.setProperty("uuid", juce::Uuid().toString(), nullptr); // new uuid for clone

        auto* self = const_cast<CompressorNode*>(this);                                        // to access processor ref
        auto clonePtr = std::make_shared<CompressorNode>(self->processor);                     // create new CompressorNode
        clonePtr->getMutableNodeState().copyPropertiesAndChildrenFrom(copiedTree, nullptr);    // copy state

        // Keep clone in processor state tree
        self->processor.apvts.state.addChild(clonePtr->getMutableNodeState(), -1, nullptr);
        clonePtr->setDisplayName(effectName); // name will be made unique in daisychain
        return clonePtr;
    }

private:
	//nodes own dsp processor + reference to main processor for param access
    AudioPluginAudioProcessor& processor;
    CompressorProcessor compressorDSP;
};