#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/PluginEditor.h"
#include <JuceHeader.h>
//ui
#include "Pitchblade/ui/TopBar.h"
#include "Pitchblade/ui/DaisyChain.h"
#include "Pitchblade/ui/EffectPanel.h"
#include "Pitchblade/ui/VisualizerPanel.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), effectPanel(p)
{
    juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    //ui//////////////////////////////////
    // all classes making the ui navigator
    addAndMakeVisible(topBar);
    addAndMakeVisible(daisyChain);
    addAndMakeVisible(effectPanel);
    addAndMakeVisible(visualizer);

    //connects daisychain to effectspanel tabs
    for (int i = 0; i < daisyChain.effectButtons.size(); ++i) {
        daisyChain.effectButtons[i]->onClick = [this, i]() {
            effectPanel.showEffect(i);
            };
    }
    //bypass button (on/off)
    topBar.bypassButton.onClick = [this]() {
        processorRef.setBypassed(!processorRef.isBypassed());
        topBar.bypassButton.setToggleState(processorRef.isBypassed(), juce::dontSendNotification);
        };
    //final size
    setSize(800, 600);
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
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //gain
    gainSlider.setBounds(getLocalBounds());

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

void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainSlider)
    {
        processorRef.gainDB = (float)gainSlider.getValue();
    }
}
