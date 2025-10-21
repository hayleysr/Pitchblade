// Written by Austin Hills

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/panels/EffectNode.h"
#include "Pitchblade/effects/DeEsserProcessor.h"

class DeEsserPanel : public juce::Component
{
public:
    explicit DeEsserPanel(AudioPluginAudioProcessor& proc);
    void resized() override;
    void paint(juce::Graphics&) override;

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
};

//Austin
class DeEsserNode : public EffectNode
{
public:
    DeEsserNode(AudioPluginAudioProcessor& proc) : EffectNode("De-Esser"), processor(proc){}

    // dsp processing step for de-esser
    void process(AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override
    {
        // get the de-esser parameters from apvts
        auto* threshold = proc.apvts.getRawParameterValue("DEESSER_THRESHOLD");
        auto* ratio = proc.apvts.getRawParameterValue("DEESSER_RATIO");
        auto* attack = proc.apvts.getRawParameterValue("DEESSER_ATTACK");
        auto* release = proc.apvts.getRawParameterValue("DEESSER_RELEASE");
        auto* frequency = proc.apvts.getRawParameterValue("DEESSER_FREQUENCY");

        //Ensure all parameters were found before using them
        if (threshold && ratio && attack && release && frequency)
        {
            proc.getDeEsserProcessor().setThreshold(threshold->load());
            proc.getDeEsserProcessor().setRatio(ratio->load());
            proc.getDeEsserProcessor().setAttack(attack->load());
            proc.getDeEsserProcessor().setRelease(release->load());
            proc.getDeEsserProcessor().setFrequency(frequency->load());
        }

        // Process the audio buffer with the updated settings
        proc.getDeEsserProcessor().process(buffer);
    }

    //return UI panel linked to node
    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override
    {
        return std::make_unique<DeEsserPanel>(proc);
    }
private:
    //nodes own dsp processor + reference to main processor for param access
    AudioPluginAudioProcessor& processor;
    DeEsserProcessor DeEsserDSP;
};