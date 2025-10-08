// austin
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"

//gain panel ui
class NoiseGatePanel : public juce::Component
{
public:
    explicit NoiseGatePanel(AudioPluginAudioProcessor& proc);
    void resized() override;
    void paint(juce::Graphics&) override;

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
    NoiseGateNode(AudioPluginAudioProcessor& proc) : EffectNode("Noise Gate"), processor(proc) {}
	// dsp processing step for noise gate
    void process(AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override {

        // get parameters from apvts
        auto* thresh = proc.apvts.getRawParameterValue("GATE_THRESHOLD");
        auto* attack = proc.apvts.getRawParameterValue("GATE_ATTACK");
        auto* release = proc.apvts.getRawParameterValue("GATE_RELEASE");
        if (thresh && attack && release)
        {
			// set parameters in dsp processor
            proc.getNoiseGateProcessor().setThreshold(thresh->load());
            proc.getNoiseGateProcessor().setAttack(attack->load());
            proc.getNoiseGateProcessor().setRelease(release->load());
        }
        proc.getNoiseGateProcessor().process(buffer);
    }

    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override {
        return std::make_unique<NoiseGatePanel>(proc);
    }

private:
    AudioPluginAudioProcessor& processor;
    NoiseGateProcessor gateDSP;
};