// reyna macabebe

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/panels/GainPanel.h"
#include "Pitchblade/panels/NoiseGatePanel.h"
#include "Pitchblade/panels/FormantPanel.h"
#include "Pitchblade/panels/PitchPanel.h"

// declare effect panels here

class AudioPluginAudioProcessor;                                            //declare your effect here
//class PitchPanel;
class FormantPanel;
//class EqualizerPanel;
//class CompressorPanel;
class GainPanel;
class NoiseGatePanel;

struct EffectDefinition {
    juce::String name;
    std::function<juce::Component* (AudioPluginAudioProcessor&)> createPanel;
};

// Global list of effects, can add more here
inline std::vector<EffectDefinition> effects = {                           // add your effect to the vector to make it appear in chain

    { "Gain", [](AudioPluginAudioProcessor& proc) -> juce::Component* {
        return new GainPanel(proc);
    }},
    { "Noise Gate",[](AudioPluginAudioProcessor& proc) -> juce::Component* {
        return new NoiseGatePanel(proc);
    } },
    { "Formant", [](auto& proc) { return new FormantPanel(proc); 
    } },
    { "Pitch", [](auto& proc) { return new PitchPanel(proc);
    } },

    { "Noise Gate",
      [](AudioPluginAudioProcessor& proc) -> juce::Component* { return new NoiseGatePanel(proc); },
      [](AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) {
          proc.getNoiseGateProcessor().setThreshold(proc.gateThresholdDb);
          proc.getNoiseGateProcessor().setAttack(proc.gateAttack);
          proc.getNoiseGateProcessor().setRelease(proc.gateRelease);
          proc.getNoiseGateProcessor().process(buffer);
      }
    },

    { "Formant",
      [](AudioPluginAudioProcessor& proc) -> juce::Component* { return new FormantPanel(proc); },
      [](AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) {
          proc.getFormantDetector().processBlock(buffer);
          if (buffer.getNumChannels() > 0) {
              auto* channelData = buffer.getReadPointer(0);
              juce::AudioBuffer<float> monoBuffer(const_cast<float**>(&channelData), 1, buffer.getNumSamples());
              proc.getFormantDetector().processBlock(monoBuffer);
              proc.getLatestFormants() = proc.getFormantDetector().getFormants();
          }
          auto freqs = proc.getFormantDetector().getFormantFrequencies();
          if (freqs.size() > 3) freqs.resize(3);
          proc.getLatestFormants() = freqs;
      }
    },

    { "Pitch",
        [](AudioPluginAudioProcessor& proc) -> juce::Component* { return new PitchPanel(proc); },
        [](AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer){
            proc.getPitchDetector().processBlock(buffer);
            proc.getPitchDetector().getCurrentPitch();
        }

    }

};
