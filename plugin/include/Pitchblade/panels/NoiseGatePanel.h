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
    NoiseGateNode(AudioPluginAudioProcessor& proc) : EffectNode(proc, "NoiseGateNode", "Noise Gate"), processor(proc) { }

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

    // return visualizer 
    std::unique_ptr<juce::Component> createVisualizer(AudioPluginAudioProcessor& proc) override {
        return std::make_unique<NoiseGateVisualizer>(proc);
    }

    ////////////////////////////////////////////////////////////

    // clone node
    std::shared_ptr<EffectNode> clone() const override { return std::make_shared<NoiseGateNode>(processor); }

private:
    //nodes own dsp processor + reference to main processor for param access
    AudioPluginAudioProcessor& processor;
    NoiseGateProcessor gateDSP;
};