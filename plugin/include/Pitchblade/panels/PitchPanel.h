/**
 * Author: Hayley Spellicy-Ryan
 */
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h" 
#include "Pitchblade/effects/PitchCorrector.h"

class PitchPanel : public juce::Component,
                   private juce::Timer          // Update UI at regular intervals
{
public:
    explicit PitchPanel(AudioPluginAudioProcessor& proc);

    void resized() override;
    void paint(juce::Graphics& g) override;
private:
    void timerCallback() override;

    void drawStaticContent(juce::Graphics& g);
    void drawDynamicLabels(juce::Graphics& g);

    juce::Rectangle<int> bounds;

    AudioPluginAudioProcessor& processor;

    juce::Label pitchName;
};
////////////////////////////////////////////////////////////
//reynas changes > added dsp node defn to ui panel creation
// pitch dsp node , processes audio + makes own panel
// inherits from EffectNode base class
#include "Pitchblade/panels/EffectNode.h"

class PitchNode : public EffectNode {
public:
    PitchNode(AudioPluginAudioProcessor& proc) : EffectNode(proc, "PitchNode", "Pitch"), processor(proc) {}

    // forward audio buffer into processor's pitch detector
    void process(AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override
    {
        //pull current note
        proc.getPitchCorrector().processBlock(buffer);   
        float pitchHz = proc.getPitchCorrector().getCurrentPitch();
    }

    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override
    {
        return std::make_unique<PitchPanel>(proc);
    }

    ////////////////////////////////////////////////////////////

    // clone node
        std::shared_ptr<EffectNode> clone() const override { return std::make_shared<PitchNode>(processor); }

private:
    AudioPluginAudioProcessor& processor;
};