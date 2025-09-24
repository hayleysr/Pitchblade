#pragma once
#include "PluginProcessor.h"

////ui
#include <JuceHeader.h>
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "Pitchblade/ui/ColorPalette.h"

#include "ui/TopBar.h"
#include "ui/DaisyChain.h"
#include "ui/EffectPanel.h"
#include "ui/VisualizerPanel.h"
#include "ui/EffectRegistry.h"

//add your effect processors here
#include "effects/GainProcessor.h"
#include "effects/NoiseGateProcessor.h"
#include "effects/FormantDetector.h"
#include "effects/PitchDetector.h"


//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

	CustomLookAndFeel customLF;     //custom colorpallet using juce lookandfeel

    //Slider for the gain
    juce::Slider gainSlider;
                                                
    TopBar topBar;
    DaisyChain daisyChain;
    EffectPanel effectPanel;
    VisualizerPanel visualizer;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};