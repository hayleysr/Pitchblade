#include "Pitchblade/ui/EffectPanel.h"

EffectPanel::EffectPanel()
{
    //adds tabs to panel
    tabs.addTab("Pitch", juce::Colours::darkgrey, new juce::Label({}, "Pitch Panel"), true);
    tabs.addTab("Formant", juce::Colours::darkgrey, new juce::Label({}, "Formant Panel"), true);
    tabs.addTab("Equalizer", juce::Colours::darkgrey, new juce::Label({}, "EQ Panel"), true);
    tabs.addTab("Compressor", juce::Colours::darkgrey, new juce::Label({}, "Comp Panel"), true);
    tabs.addTab("Gate/Gain", juce::Colours::darkgrey, new juce::Label({}, "Noise/Gain Panel"), true);
    //hides top tabs
    tabs.setTabBarDepth(0);
    addAndMakeVisible(tabs);
}

void EffectPanel::resized()
{
    tabs.setBounds(getLocalBounds());
}

void EffectPanel::showEffect(int index)
{
    //switches current tab
    tabs.setCurrentTabIndex(index);
}