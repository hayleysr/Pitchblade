// austins code
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"

//gain panel ui
// creates slider n attaches to apvts parameter (in pluginprocessor.cpp)
class GainPanel : public juce::Component {
public:
    explicit GainPanel(AudioPluginAudioProcessor& proc);
    void resized() override;
    void paint(juce::Graphics&) override; 

private:
    //declaring slider n processor
    AudioPluginAudioProcessor& processor;
    //ui
    juce::Slider gainSlider;
    juce::Label gainLabel;

	// attachment to link slider to the apvts parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
};

////////////////////////////////////////////////////////////

//reynas changes > added dsp node defn to ui panel creation
// gain dsp node , processes audio + makes own panel
// inherits from EffectNode base class
#include "Pitchblade/panels/EffectNode.h"
#include "Pitchblade/effects/GainProcessor.h"

class GainNode : public EffectNode 
{
public:
    //GainNode() : EffectNode("Gain") {}
    GainNode(AudioPluginAudioProcessor& proc) : EffectNode("Gain"), processor(proc) { }

    // dsp processing step for gain
    void process(AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override {

        // get the gain parameter from apvts
        auto* val = proc.apvts.getRawParameterValue("GAIN");
        if (val != nullptr)
            proc.getGainProcessor().setGain(val->load());
            proc.getGainProcessor().process(buffer);
    }

    // return UI panel linked to node
    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override {
        return std::make_unique<GainPanel>(proc);
    }

private:
	//nodes own dsp processor + reference to main processor for param access
    AudioPluginAudioProcessor& processor;
    GainProcessor gainDSP;
};