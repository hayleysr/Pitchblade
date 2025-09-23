#include <juce_core/juce_core.h>  // Or just include juce/juce.h

#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/PluginEditor.h"
#include <JuceHeader.h>

// ui
#include "Pitchblade/ui/TopBar.h"
#include "Pitchblade/ui/DaisyChain.h"
#include "Pitchblade/ui/EffectPanel.h"
#include "Pitchblade/ui/VisualizerPanel.h"


//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), effectPanel(p)
{
    //juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

     /*gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
     gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
     gainSlider.setRange(-48.0, 48.0);
     gainSlider.setValue(0.0);
     gainSlider.addListener(this);
     addAndMakeVisible(gainSlider);*/

    // startTimerHz(30); // repaint timer 30 times per second - huda

    //// Formant toggle button - huda
    //toggleViewButton.addListener(this);
    //addAndMakeVisible(toggleViewButton);

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
    //toggleViewButton.removeListener(this);
    //gainSlider.removeListener(this);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour) default background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));  
    g.drawText(std::to_string(processorRef.getPitchDetector().getCurrentPitch()), 50, 50, 50, 50, juce::Justification::centred);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    //toggleViewButton.setBounds(10, 10, 120, 30);// huda

    //if (!showingFormants) // So that gain GUI isn't affected much - huda
    //{
    //    gainSlider.setBounds(getLocalBounds().reduced(50));
    //    gainSlider.setVisible(true);
    //}
    //else
    //{
    //    gainSlider.setVisible(false);
    //}

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
//void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
//{
//    if (slider == &gainSlider)
//    {
//        processorRef.gainDB = (float)gainSlider.getValue();
//    }
//}

////Button to show formants vs gain - huda
//void AudioPluginAudioProcessorEditor::buttonClicked(juce::Button* button)
//{
//    if (button == &toggleViewButton)
//    {
//        showingFormants = !showingFormants;
//        toggleViewButton.setButtonText(showingFormants ? "Show Gain" : "Show Formants");
//        resized();
//        repaint();
//    }
//}

//void AudioPluginAudioProcessorEditor::timerCallback() {
//    if (showingFormants) { // refresh if formants are visible
//        repaint();
//    }
//}
