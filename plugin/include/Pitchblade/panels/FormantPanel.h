//hudas code
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/effects/FormantDetector.h"
#include "Pitchblade/effects/FormantShifter.h"
#ifndef PARAM_FORMANT_SHIFT
  #define PARAM_FORMANT_SHIFT "FORMANT_SHIFT"
#endif
#ifndef PARAM_FORMANT_MIX
  #define PARAM_FORMANT_MIX "FORMANT_MIX"
#endif

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
        // --- ADD: Formant Shifter controls
    juce::Label  formantLabel, mixLabel;
    juce::Slider formantSlider, mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> formantAttach, mixAttach;

    // Optional: drawing area for detector overlay below the sliders
    juce::Rectangle<int> detectorArea;
};

////////////////////////////////////////////////////////////

//reynas changes > added dsp node defn to ui panel creation
// formant dsp node , processes audio + makes own panel
// inherits from EffectNode base class
#include "Pitchblade/panels/EffectNode.h"

class FormantNode : public EffectNode {
public:
    FormantNode(AudioPluginAudioProcessor& proc): EffectNode(proc, "FormantNode", "Formant"), processor(proc) {}

    // forward buffer into processor's formant detector
   void process (AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) override
    {
        // --- 0) Grab params
        const float shift = proc.apvts.getRawParameterValue(PARAM_FORMANT_SHIFT)->load();
        const float mix   = proc.apvts.getRawParameterValue(PARAM_FORMANT_MIX  )->load();

        // --- 1) DRY analysis (for shifter control)
        juce::AudioBuffer<float> dry; dry.makeCopyOf(buffer, true);
        proc.getFormantDetector().processBlock(dry);

        auto freqsDry = proc.getFormantDetector().getFormantFrequencies();
        std::sort(freqsDry.begin(), freqsDry.end());
        // keep detector feed broad enough for real F1
        freqsDry.erase(std::remove_if(freqsDry.begin(), freqsDry.end(),
                                    [](float f){ return f < 120.f || f > 3500.f; }),
                    freqsDry.end());
        if (freqsDry.size() > 2) freqsDry.resize(2);

        // --- 2) Process (shifter uses DRY peaks)
        if (!bypassed)
        {
            auto& sh = proc.getFormantShifter();
            sh.setFormantFrequencies(0, freqsDry);
            if (buffer.getNumChannels() > 1) sh.setFormantFrequencies(1, freqsDry);
            sh.setShiftAmount(shift);
            sh.setMix(mix);
            sh.processBlock(buffer);
        }

        // --- 3) WET analysis purely for the PANEL display (so lines move!)
        proc.getFormantDetector().processBlock(buffer); // analyze processed audio for UI
        auto freqsWet = proc.getFormantDetector().getFormantFrequencies();

        // For the panel, hide F0/harmonics so you don’t see a “stuck” line:
        std::sort(freqsWet.begin(), freqsWet.end());
        freqsWet.erase(std::remove_if(freqsWet.begin(), freqsWet.end(),
                                    [](float f){ return f < 300.f || f > 5000.f; }), // panel-only view
                    freqsWet.end());
        if (freqsWet.size() > 3) freqsWet.resize(3);

        proc.setLatestFormants(freqsWet);
    }




    std::unique_ptr<juce::Component> createPanel(AudioPluginAudioProcessor& proc) override {
        return std::make_unique<FormantPanel>(proc);
    }

    ////////////////////////////////////////////////////////////

    // clone node
        std::shared_ptr<EffectNode> clone() const override
        {
            return std::make_shared<FormantNode>(processor);
        }

private:
    AudioPluginAudioProcessor& processor;
};