#pragma once
#include "PluginProcessor.h"

//ui
#include <JuceHeader.h>
#include "ui/TopBar.h"
#include "ui/DaisyChain.h"
#include "ui/EffectPanel.h"
#include "ui/VisualizerPanel.h"

#include "Pitchblade/ui/EffectRegistry.h"
#include "Pitchblade/GainProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              public juce::Slider::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    juce::Slider gainSlider;

    //ui declaration, defined in indiv classes
    TopBar topBar;
    DaisyChain daisyChain;
    EffectPanel effectPanel;
    VisualizerPanel visualizer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};

//