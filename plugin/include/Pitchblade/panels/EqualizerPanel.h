#pragma once
// Author: huda
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/panels/EffectNode.h"

// ===================== Panel (UI) =====================
class EqualizerPanel : public juce::Component
{
public:
    explicit EqualizerPanel (AudioPluginAudioProcessor& proc);
    void resized() override;

private:
    static void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& text,
                           double min, double max, double step, bool isGain);

    AudioPluginAudioProcessor& processor;

    juce::Slider lowFreq,  lowGain,
                 midFreq,  midGain,
                 highFreq, highGain;

    juce::Label  lowFreqLabel,  lowGainLabel,
                 midFreqLabel,  midGainLabel,
                 highFreqLabel, highGainLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment
        lowFreqAttachment,  lowGainAttachment,
        midFreqAttachment,  midGainAttachment,
        highFreqAttachment, highGainAttachment;
};

// ===================== Node =====================
class EqualizerNode : public EffectNode
{
public:
    explicit EqualizerNode (AudioPluginAudioProcessor& proc);
    EqualizerNode (AudioPluginAudioProcessor& proc, const juce::ValueTree& state);

    // UI creator
    std::unique_ptr<juce::Component> createPanel (AudioPluginAudioProcessor& proc) override;

    // DSP step
    void process (AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override;

    // Clone node
    std::shared_ptr<EffectNode> clone() const override;
};