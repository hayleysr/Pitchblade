#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), effectPanel(p)
{
    juce::ignoreUnused (processorRef);
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
        processorRef.setBypassed(!processorRef.isBypassed());
        topBar.bypassButton.setToggleState(processorRef.isBypassed(), juce::dontSendNotification);
        };

}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour) default background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));  
}

void AudioPluginAudioProcessorEditor::resized()
{
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
//This function checks to see if any slider's value has changed
void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    
}