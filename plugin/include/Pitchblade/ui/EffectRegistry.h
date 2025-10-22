// reyna

#pragma once
#include <JuceHeader.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Pitchblade/panels/GainPanel.h"
#include "Pitchblade/panels/NoiseGatePanel.h"
#include "Pitchblade/panels/FormantPanel.h"
#include "Pitchblade/panels/PitchPanel.h"
#include "Pitchblade/ui/DaisyChainItem.h"

class AudioPluginAudioProcessor;   

//class PitchPanel;
//class EqualizerPanel;
//class CompressorPanel;
class GainPanel;
class NoiseGatePanel;
class FormantPanel; 

struct EffectDefinition {
    juce::String name;
    std::function<juce::Component* (AudioPluginAudioProcessor&)> createPanel;               
    std::function<void(AudioPluginAudioProcessor&, juce::AudioBuffer<float>&)> process;     //dsp callback
    bool bypassed = false;                                                                  //bypass checker
};

// Global list of effects, can add more here
inline std::vector<EffectDefinition> effects = {

    { "Gain",
      [](AudioPluginAudioProcessor& proc) -> juce::Component* { return new GainPanel(proc); },
      [](AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) {
        // Fetch parameter from APVTS
        auto* val = proc.apvts.getRawParameterValue("GAIN");
        if (val != nullptr)
            proc.getGainProcessor().setGain(val->load());
            proc.getGainProcessor().process(buffer);
      }
    },

    { "Noise Gate",
      [](AudioPluginAudioProcessor& proc) -> juce::Component* { return new NoiseGatePanel(proc); },
      [](AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer) {
         // Fetch parameter from APVTS
         auto* thresh = proc.apvts.getRawParameterValue("GATE_THRESHOLD");
         auto* attack = proc.apvts.getRawParameterValue("GATE_ATTACK");
         auto* release = proc.apvts.getRawParameterValue("GATE_RELEASE");
         if (thresh && attack && release)
         {
            proc.getNoiseGateProcessor().setThreshold(thresh->load());
            proc.getNoiseGateProcessor().setAttack(attack->load());
            proc.getNoiseGateProcessor().setRelease(release->load());
         }
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
            DBG("Pitch correction START - Buffer RMS: " << buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
            proc.getPitchCorrector().processBlock(buffer);
            proc.getPitchCorrector().getCurrentPitch();
            DBG("Pitch correction END - Buffer RMS: " << buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
            DBG("Detected Pitch: " << proc.getPitchCorrector().getCurrentPitch());
            DBG("Correction State: " << (int)proc.getPitchCorrector().getCurrentState());
        }

    }

};