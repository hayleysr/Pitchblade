#include <juce_core/juce_core.h>  // Or just include juce/juce.h
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/PluginEditor.h"
#include "Pitchblade/ui/ColorPalette.h"


// ui
#include "Pitchblade/ui/TopBar.h"
#include "Pitchblade/ui/DaisyChain.h"
#include "Pitchblade/ui/EffectPanel.h"
#include "Pitchblade/ui/VisualizerPanel.h"
#include "Pitchblade/ui/EffectRegistry.h"
#include "Pitchblade/ui/DaisyChainItem.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
                                                                : AudioProcessorEditor(&p), processorRef(p), effectPanel(p)
{   // gui frontend / ui reyna
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(nullptr),
    setSize (800, 600);
	setLookAndFeel(&customLF);  //apply custom look and feel globally

    addAndMakeVisible(topBar);
    addAndMakeVisible(daisyChain);
    addAndMakeVisible(effectPanel);
    addAndMakeVisible(visualizer);

    // Top bar bypass 
    topBar.bypassButton.setClickingTogglesState(false);
    topBar.bypassButton.onClick = [this]() {
        const bool newState = !processorRef.isBypassed();
        processorRef.setBypassed(newState);

		//update topbar bypass button color
        const auto bg = newState ? Colors::accent : Colors::panel;
            topBar.bypassButton.setColour(juce::TextButton::buttonColourId, bg);
            topBar.bypassButton.setColour(juce::TextButton::buttonOnColourId, bg); 
            topBar.bypassButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            topBar.bypassButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white); 
            topBar.bypassButton.repaint();
			//update daisychain bypass buttons color : greyed out
            daisyChain.setGlobalBypassVisual(newState);
        };

	//connect DaisyChain buttons to EffectPanel
    for (int i = 0; i < daisyChain.items.size(); ++i)
    {
        daisyChain.items[i]->button.onClick = [this, i]()
            {
                effectPanel.showEffect(i);
            };
    }
    //keeps daiychain reordering consistant
    daisyChain.onReorderFinished = [this]()
        {
            effectPanel.refreshTabs();
            for (int i = 0; i < daisyChain.items.size(); ++i)
            {
                daisyChain.items[i]->button.onClick = [this, i]()
                    {
                        effectPanel.showEffect(i); 
                    };
            }
        };
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
	setLookAndFeel(nullptr); //reset look and feel
}

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
    
    //ui reyna//////////////////////////////////////////
    auto area = getLocalBounds();
    //topbar height
    auto top = area.removeFromTop(40);
    topBar.setBounds(top);
    //daisychain width
    auto left = area.removeFromLeft(190);
    daisyChain.setBounds(left);
    //effects panel
    auto center = area.removeFromTop(area.getHeight() / 2);
    effectPanel.setBounds(center);
    //visualizer
    visualizer.setBounds(area);


}
