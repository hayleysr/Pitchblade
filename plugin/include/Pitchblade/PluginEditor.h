#pragma once
#include "PluginProcessor.h"
#include "Pitchblade/FormantDetector.h"

//ui
#include <JuceHeader.h>
#include "ui/TopBar.h"
#include "ui/DaisyChain.h"
#include "ui/EffectPanel.h"
#include "ui/VisualizerPanel.h"
#include "ui/EffectRegistry.h"

//add your effect processors here
#include "effects/GainProcessor.h"
#include "effects/NoiseGateProcessor.h"


//ui
#include <JuceHeader.h>
#include "ui/TopBar.h"
#include "ui/DaisyChain.h"
#include "ui/EffectPanel.h"
#include "ui/VisualizerPanel.h"

#include "Pitchblade/ui/EffectRegistry.h"
#include "Pitchblade/effects/GainProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              public juce::Slider::Listener,
                                              public juce::Button::Listener,  //To handle button clicks - huda
                                              private juce::Timer //adding a timer to update formants - huda
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
                                                
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override; // Handle toggle button clicks - huda
    void timerCallback() override; //huda

    //void sliderValueChanged(juce::Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    //Slider for the gain
    juce::Slider gainSlider;

                                                
    // For formant display - huda
    juce::TextButton toggleViewButton { "Show Formants" }; // Button to switch views - huda
    bool showingFormants = false;// True if formant UI is active - huda
                                                
    TopBar topBar;
    DaisyChain daisyChain;
    EffectPanel effectPanel;
    VisualizerPanel visualizer;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};