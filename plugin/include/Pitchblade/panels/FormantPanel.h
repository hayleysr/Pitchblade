//hudas code
#pragma once
#include <JuceHeader.h>
#include <algorithm>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/effects/FormantDetector.h"
#include "Pitchblade/effects/FormantShifter.h"
#include "Pitchblade/ui/FormantVisualizer.h"

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

    bool showingFormants = true;

    juce::TextButton toggleViewButton{ "Show Formants" };
    juce::Slider gainSlider;

    // --- Formant Shifter controls
    juce::Label  formantLabel, mixLabel;
    juce::Slider formantSlider, mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> formantAttach, mixAttach;

    // Optional: drawing area for detector overlay below the sliders
    juce::Rectangle<int> detectorArea;

    // Formant visualizer component
    std::unique_ptr<FormantVisualizer> formantVisualizer;
};

////////////////////////////////////////////////////////////

// reynas changes > added dsp node defn to ui panel creation
// formant dsp node , processes audio + makes own panel
// inherits from EffectNode base class
class FormantNode : public EffectNode
{
public:
    FormantNode (AudioPluginAudioProcessor& proc)
        : EffectNode (proc, "FormantNode", "Formant"), processor (proc) {}

    void process (AudioPluginAudioProcessor& proc,
                  juce::AudioBuffer<float>& buffer) override
    {
        // --- 0) Grab UI params
        const auto* shiftParam = proc.apvts.getRawParameterValue (PARAM_FORMANT_SHIFT);
        const auto* mixParam   = proc.apvts.getRawParameterValue (PARAM_FORMANT_MIX);

        float shift = shiftParam ? shiftParam->load() : 0.0f;
        float mix   = mixParam   ? mixParam->load()   : 1.0f;   // 0=dry, 1=wet

        // If this node is bypassed, force neutral formant and fully dry
        if (bypassed)
        {
            shift = 0.0f;
            mix   = 0.0f;
        }

        // Get shifter + detector
        auto& sh = proc.getFormantShifter();
        auto& det = proc.getFormantDetector();

        // Make sure shifter sees the current amount
        sh.setShiftAmount (shift);   // [-50..50] -> internal ratio

        const int numCh = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        if (numCh == 0 || numSamples == 0)
            return;

        // Copy dry input
        dryBuffer.setSize (numCh, numSamples, false, false, true);
        dryBuffer.makeCopyOf (buffer);

        // Process in-place to get the wet signal
        sh.processBlock (buffer); // buffer = wet

        //Crossfade dry/wet into buffer
        const float wetGain  = juce::jlimit (0.0f, 1.0f, mix);
        const float dryGain  = 1.0f - wetGain;

        for (int ch = 0; ch < numCh; ++ch)
        {
            const float* dry = dryBuffer.getReadPointer (ch);
            float* wet = buffer.getWritePointer (ch);

            for (int n = 0; n < numSamples; ++n)
                wet[n] = dryGain * dry[n] + wetGain * wet[n];
        }

        //Analyze the post-mix output for the panel
        det.processBlock (buffer);
        auto freqsWet = det.getFormantFrequencies();

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

    std::shared_ptr<EffectNode> clone() const override
    {
        return std::make_shared<FormantNode> (processor);
    }

    std::unique_ptr<juce::XmlElement> toXml() const override;
    void loadFromXml (const juce::XmlElement& xml) override;

private:
    AudioPluginAudioProcessor& processor;

    // --- new: buffer to hold the dry input for dry/wet mixing
    juce::AudioBuffer<float> dryBuffer;
};
