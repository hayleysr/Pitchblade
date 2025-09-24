#include <juce_core/juce_core.h>  // Or just include juce/juce.h
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/PluginEditor.h"


// ui
#include "Pitchblade/ui/TopBar.h"
#include "Pitchblade/ui/DaisyChain.h"
#include "Pitchblade/ui/EffectPanel.h"
#include "Pitchblade/ui/VisualizerPanel.h"
#include "Pitchblade/ui/EffectRegistry.h"


//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), effectPanel(p)
{   // gui frontend / ui
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize (800, 600);

    addAndMakeVisible(topBar);
    addAndMakeVisible(daisyChain);
    addAndMakeVisible(effectPanel);
    addAndMakeVisible(visualizer);


    // connect DaisyChain buttons to EffectPanel
    for (int i = 0; i < daisyChain.effectButtons.size(); ++i) {
        daisyChain.effectButtons[i]->onClick = [this, i]() {
            effectPanel.showEffect(i);
            };
    }

    // bypass button
    topBar.bypassButton.onClick = [this]() {
        processorRef.setBypassed(!processorRef.isBypassed());       //sets bypass state
        topBar.bypassButton.setToggleState(processorRef.isBypassed(), juce::dontSendNotification);
        };

}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() = default;

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    //ui//////////////////////////////////////////
    auto area = getLocalBounds();
    //topbar height
    auto top = area.removeFromTop(40);
    topBar.setBounds(top);
    //daisychain width
    auto left = area.removeFromLeft(150);
    daisyChain.setBounds(left);
    //effects panel
    auto center = area.removeFromTop(area.getHeight() / 2);
    effectPanel.setBounds(center);
    //visualizer
    visualizer.setBounds(area);


}
