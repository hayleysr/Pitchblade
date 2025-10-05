//hudas code
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/effects/FormantDetector.h"

class FormantPanel : public juce::Component,
                     public juce::Button::Listener,     //To handle button clicks - huda
                     private juce::Timer                //adding a timer to update formants - huda
{
public:
    explicit FormantPanel(AudioPluginAudioProcessor& proc);
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;

    AudioPluginAudioProcessor& processor;


    bool showingFormants = false;

    juce::TextButton toggleViewButton{ "Show Formants" };
    juce::Slider gainSlider;

};

////////////////////////////////////////////////////////////

//reynas changes > added dsp node defn to ui panel creation
// formant dsp node , processes audio + makes own panel
// inherits from EffectNode base class
#include "Pitchblade/panels/EffectNode.h"

class FormantNode : public EffectNode {
public:
    FormantNode(AudioPluginAudioProcessor& proc) : EffectNode("Formant"), processor(proc) { }

    // forward buffer into processor's formant detector
    void process(AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override {
		//pull current formants
        proc.getFormantDetector().processBlock(buffer);

        if (buffer.getNumChannels() > 0) {
            auto* channelData = buffer.getReadPointer(0);
            juce::AudioBuffer<float> monoBuffer(const_cast<float**>(&channelData), 1, buffer.getNumSamples());
            proc.getFormantDetector().processBlock(monoBuffer); // run analysis again on mono
        }

        // overwrite with top frequencies
        auto freqs = proc.getFormantDetector().getFormantFrequencies();

        // Keep only top 3
        if (freqs.size() > 3) freqs.resize(3);
        // Save into processor for panel
        proc.setLatestFormants(freqs);
    }

    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override {
        return std::make_unique<FormantPanel>(proc);
    }

private:
    AudioPluginAudioProcessor& processor;
};
