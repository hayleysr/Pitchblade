/**
 * Authors: Hayley Spellicy-Ryan, Reyna Macabebe
 */
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h" 
#include "Pitchblade/effects/PitchCorrector.h"
#include "Pitchblade/ui/LevelMeter.h"

class PitchPanel : public juce::Component,
                   private juce::Timer,          // Update UI at regular intervals
                   public juce::ValueTree::Listener
{
public:
    explicit PitchPanel(AudioPluginAudioProcessor& proc, juce::ValueTree& state);

    void resized() override;
    void paint(juce::Graphics& g) override;
    
    // callback override for value tree
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

private:
    void timerCallback() override;

    void drawStaticContent(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawDynamicLabels(juce::Graphics& g, juce::Rectangle<float> bounds);

    std::unique_ptr<LevelMeter> leftLevelMeter, rightLevelMeter;

    juce::Slider retuneSlider, noteTransitionSlider, smoothingSlider, waverSlider;
    juce::Label retuneLabel, noteTransitionLabel, smoothingLabel, waverLabel;
    juce::ComboBox scaleOffsetBox, scaleTypeBox;

    // Link to APVTS
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> retuneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noteTransitionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> smoothingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> waverAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleOffsetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleTypeAttachment;

    juce::ValueTree localState;

    AudioPluginAudioProcessor& processor;

    juce::Label pitchName;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchPanel)
};
////////////////////////////////////////////////////////////
//reynas changes > added dsp node defn to ui panel creation
// pitch dsp node , processes audio + makes own panel
// inherits from EffectNode base class
//hayley's changes > added value tree stuff, copying austin's formatting
#include "Pitchblade/panels/EffectNode.h"

class PitchNode : public EffectNode {
public:
    explicit PitchNode(AudioPluginAudioProcessor& proc) : EffectNode(proc, "PitchNode", "Pitch"), processor(proc) {
        if (!getMutableNodeState().hasProperty("PitchRetune"))
            getMutableNodeState().setProperty("PitchRetune", 0.3f, nullptr);
        if (!getMutableNodeState().hasProperty("PitchNoteTransition"))
            getMutableNodeState().setProperty("PitchNoteTransition", 50.f, nullptr);
        if (!getMutableNodeState().hasProperty("PitchSmoothing"))
            getMutableNodeState().setProperty("PitchSmoothing", 1.f, nullptr);
        if (!getMutableNodeState().hasProperty("PitchWaver"))
            getMutableNodeState().setProperty("PitchWaver", 0.f, nullptr);

        if (!getMutableNodeState().hasProperty("PitchOffset"))
            getMutableNodeState().setProperty("PitchOffset", 0, nullptr);
        if (!getMutableNodeState().hasProperty("PitchType"))
            getMutableNodeState().setProperty("PitchType", 0, nullptr);

        if (!processor.apvts.state.hasType("EffectNodes"))
            processor.apvts.state = juce::ValueTree("EffectNodes");

        processor.apvts.state.addChild(getMutableNodeState(), -1, nullptr);

        pitchDSP.prepare(proc.getSampleRate(), proc.getBlockSize());

    }

    // forward audio buffer into processor's pitch detector
    void process(AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override
    {
        juce::ignoreUnused(proc);
        
        const float retuneSpeed = (float)getNodeState().getProperty("PitchRetune", 0.3f);
        const float noteTransition = (float)getNodeState().getProperty("PitchNoteTransition", 50.f);
        const float smoothing = (float)getNodeState().getProperty("PitchSmoothing", 1.f);
        const float waver = (float)getNodeState().getProperty("PitchWaver", 0.f);
        const int offset = (int)getNodeState().getProperty("PitchOffset", 0);
        const int type = (int)getNodeState().getProperty("PitchType", 0);

        pitchDSP.setRetuneSpeed(retuneSpeed);
        pitchDSP.setNoteTransition(noteTransition);
        pitchDSP.setCorrectionRatio(smoothing);
        pitchDSP.setWaver(waver);
        pitchDSP.setScaleOffset(offset);
        pitchDSP.setScaleType(type);

        //pull current note
        proc.getPitchCorrector().processBlock(buffer);   
        float pitchHz = proc.getPitchCorrector().getCurrentPitch();
        pitchDSP.currentOutputPitch.store(pitchHz);
    }

    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override
    {
        return std::make_unique<PitchPanel>(proc, getMutableNodeState());
    }

    std::atomic<float>& getPitchAtomic(){
        return pitchDSP.currentOutputPitch;
    }

    //////////////////////////////////////////////////////////// reyna

    // clone node >  copied from austin
    std::shared_ptr<EffectNode> clone() const override { 
        auto copiedTree = getNodeState().createCopy();                    // Copy ValueTree state
        copiedTree.setProperty("uuid", juce::Uuid().toString(), nullptr); // new uuid for clone

        auto* self = const_cast<PitchNode*>(this);                                        // to access processor ref
        auto clonePtr = std::make_shared<PitchNode>(self->processor);                     // create new CompressorNode
        clonePtr->getMutableNodeState().copyPropertiesAndChildrenFrom(copiedTree, nullptr);    // copy state

        // Keep clone in processor state tree
        self->processor.apvts.state.addChild(clonePtr->getMutableNodeState(), -1, nullptr);
        clonePtr->setDisplayName(effectName); // name will be made unique in daisychain
        return clonePtr;
    }

        // XML serialization for saving/loading
        std::unique_ptr<juce::XmlElement> toXml() const override;
        void loadFromXml(const juce::XmlElement& xml) override;

private:
    AudioPluginAudioProcessor& processor;
    PitchCorrector pitchDSP;
};