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
                     public juce::Button::Listener,     // To handle button clicks - huda
                     private juce::Timer                 // adding a timer to update formants - huda
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

    // --- Formant Shifter controls
    juce::Label  formantLabel, mixLabel;
    juce::Slider formantSlider, mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> formantAttach, mixAttach;

    // Optional: drawing area for detector overlay below the sliders
    juce::Rectangle<int> detectorArea;
};

////////////////////////////////////////////////////////////

// reynas changes > added dsp node defn to ui panel creation
// formant dsp node , processes audio + makes own panel
// inherits from EffectNode base class
#include "Pitchblade/panels/EffectNode.h"

class FormantNode : public EffectNode
{
public:
    FormantNode (AudioPluginAudioProcessor& proc)
        : EffectNode (proc, "FormantNode", "Formant"), processor (proc) {}

    void process (AudioPluginAudioProcessor& proc,
                  juce::AudioBuffer<float>& buffer) override
    {
        // --- 0) Grab UI param
        float shift = proc.apvts.getRawParameterValue (PARAM_FORMANT_SHIFT)->load();

        // If this node is bypassed, force neutral formant (but still run shifter
        // so RubberBand’s internal latency/FIFO stay consistent)
        if (bypassed)
            shift = 0.0f;

        // --- 1) Process with RubberBand-based FormantShifter (formant only)
        auto& sh = proc.getFormantShifter();   // your new RubberBand FormantShifter
        sh.setShiftAmount (shift);            // [-50..50] -> ratio internally
        sh.processBlock (buffer);             // in-place

        // --- 2) Analyze the *current* buffer for panel display (post-effect)
        auto& det = proc.getFormantDetector();
        det.processBlock (buffer);
        auto freqsWet = det.getFormantFrequencies();

        // For the panel, hide F0/harmonics so you don’t see a “stuck” line:
        std::sort (freqsWet.begin(), freqsWet.end());
        freqsWet.erase (std::remove_if (freqsWet.begin(), freqsWet.end(),
                                        [] (float f) { return f < 300.f || f > 5000.f; }),
                        freqsWet.end());
        if (freqsWet.size() > 3)
            freqsWet.resize (3);

        proc.setLatestFormants (freqsWet);
    }

    std::unique_ptr<juce::Component> createPanel (AudioPluginAudioProcessor& proc) override
    {
        return std::make_unique<FormantPanel> (proc);
    }

    // clone node
    std::shared_ptr<EffectNode> clone() const override
    {
        return std::make_shared<FormantNode> (processor);
    }

    // XML serialization for saving/loading
    std::unique_ptr<juce::XmlElement> toXml() const override;
    void loadFromXml (const juce::XmlElement& xml) override;

private:
    AudioPluginAudioProcessor& processor;
};