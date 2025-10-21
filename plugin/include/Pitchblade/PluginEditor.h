#pragma once
#include "PluginProcessor.h"

////ui
#include <JuceHeader.h>
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/TooltipManager.h"

#include "ui/TopBar.h"
#include "ui/DaisyChain.h"
#include "ui/EffectPanel.h"
#include "ui/VisualizerPanel.h"

#include "Pitchblade/panels/EffectNode.h"


//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              public juce::DragAndDropContainer
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    //reyna ui
	CustomLookAndFeel customLF;     //custom colorpallet using juce lookandfeel
                                                
    TopBar topBar;
    DaisyChain daisyChain;
    EffectPanel effectPanel;
    VisualizerPanel visualizer;

    TooltipManager tooltipManager;
    std::unique_ptr<juce::TooltipWindow> tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};