// reyna macabebe

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/panels/GainPanel.h"


// declare effect panels here
class AudioPluginAudioProcessor;
//class PitchPanel;
//class FormantPanel;
//class EqualizerPanel;
//class CompressorPanel;
class GainPanel;
// reyna macabebe
struct EffectDefinition {
    juce::String name;
    std::function<juce::Component* (AudioPluginAudioProcessor&)> createPanel;
};

// Global list of effects, can add more here
inline std::vector<EffectDefinition> effects = {
    //{ "Pitch",      [](auto& proc) { return new PitchPanel(proc); } },
    //{ "Formant",    [](auto& proc) { return new FormantPanel(proc); } },
    //{ "Equalizer",  [](auto& proc) { return new EqualizerPanel(proc); } },
    //{ "Compressor", [](auto& proc) { return new CompressorPanel(proc); } },
    { "Gate/Gain", [](AudioPluginAudioProcessor& proc) -> juce::Component* {
        return new GainPanel(proc);
    }},
    //test secondary gain, shows how each one is an individial item
    { "Gate/Gain", [](AudioPluginAudioProcessor& proc) -> juce::Component* {
        return new GainPanel(proc);
    }}

};
