// reyna macabebe

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/panels/GainPanel.h"
#include "Pitchblade/panels/NoiseGatePanel.h"
#include "Pitchblade/panels/FormantPanel.h"

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

    //test secondary gain, shows how each one is an individial item
    //{ "test", [](AudioPluginAudioProcessor& proc) -> juce::Component* {
    //    return new GainPanel(proc); //gain for testing duplicates
    //}}
};
