// Written by Austin Hills

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"

//Defines the UI panel
class CompressorPanel : public juce::Component
{
public:
    explicit CompressorPanel(AudioPluginAudioProcessor& proc);
    void resized() override;
    void paint(juce::Graphics&) override;

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

    //Update visibility function
    void updateSliderVisibility();
};

////////////////////////////////////////////////////////////

//Austin (copying reyna's formatting)
#include "Pitchblade/panels/EffectNode.h"
#include "Pitchblade/effects/CompressorProcessor.h"

class CompressorNode : public EffectNode 
{
public:
    //GainNode() : EffectNode("Gain") {}
    CompressorNode(AudioPluginAudioProcessor& proc) : EffectNode("Compressor"), processor(proc) { }

    // dsp processing step for compressor
    void process(AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override {

        // get the compressor parameters from apvts
        auto* threshold = proc.apvts.getRawParameterValue("COMP_THRESHOLD");
        auto* ratio = proc.apvts.getRawParameterValue("COMP_RATIO");
        auto* attack = proc.apvts.getRawParameterValue("COMP_ATTACK");
        auto* release = proc.apvts.getRawParameterValue("COMP_RELEASE");
        auto* limiter = proc.apvts.getRawParameterValue("COMP_LIMITER_MODE");
        
        //Ensure all parameters were found before using them
        if (threshold && ratio && attack && release && limiter){
            //Check if limiter is active
            bool isLimiterMode = limiter->load() > 0.5f;

            if(!isLimiterMode){
                //In limiter mode, use a high fixed ratio and fast attack
                proc.getCompressorProcessor().setThreshold(threshold->load());
                proc.getCompressorProcessor().setRatio(2000.0f); //High, fixed ratio
                proc.getCompressorProcessor().setAttack(1.0f); //Very fast attack
                proc.getCompressorProcessor().setRelease(release->load());
            }else{
                //In simple compressor mode, use the values from the sliders
                proc.getCompressorProcessor().setThreshold(threshold->load());
                proc.getCompressorProcessor().setRatio(ratio->load()); 
                proc.getCompressorProcessor().setAttack(attack->load()); 
                proc.getCompressorProcessor().setRelease(release->load());
            }
        }

        //Process the audio buffer with the updated settings
            proc.getCompressorProcessor().process(buffer);
    }

    // return UI panel linked to node
    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override {
        return std::make_unique<CompressorPanel>(proc);
    }

private:
	//nodes own dsp processor + reference to main processor for param access
    AudioPluginAudioProcessor& processor;
    CompressorProcessor compressorDSP;
};