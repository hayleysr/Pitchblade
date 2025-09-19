#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);

    // GAIN SECTION AUSTIN HILLS
    //gain slider
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 25);
    gainSlider.setRange(-48.0, 48.0);
    gainSlider.setValue(processorRef.gainDB);
    gainSlider.addListener(this);
    addAndMakeVisible(gainSlider);

    //gain label
    gainLabel.setText("Gain", juce::dontSendNotification);
    addAndMakeVisible(gainLabel);

    //NOISE GATE SECTION AUSTIN HILLS
    //threshold slider
    noiseGateThresholdSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    noiseGateThresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 25);
    noiseGateThresholdSlider.setRange(-60.0, 0.0);
    noiseGateThresholdSlider.setValue(processorRef.gateThresholdDb);
    noiseGateThresholdSlider.addListener(this);
    addAndMakeVisible(noiseGateThresholdSlider);

    //attack slider
    noiseGateAttackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    noiseGateAttackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 25);
    noiseGateAttackSlider.setRange(0.1, 2000.0);
    noiseGateAttackSlider.setValue(processorRef.gateAttack);
    noiseGateAttackSlider.addListener(this);
    addAndMakeVisible(noiseGateAttackSlider);

    //attack slider
    noiseGateReleaseSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    noiseGateReleaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 25);
    noiseGateReleaseSlider.setRange(0.1, 2000.0);
    noiseGateReleaseSlider.setValue(processorRef.gateRelease);
    noiseGateReleaseSlider.addListener(this);
    addAndMakeVisible(noiseGateReleaseSlider);

    //noise gate label
    noiseGateLabel.setText("Noise Gate", juce::dontSendNotification);
    addAndMakeVisible(noiseGateLabel);

}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    
}

void AudioPluginAudioProcessorEditor::resized()
{
    //Splitting the screen vertically to have two sections
    auto noiseGateSection = getLocalBounds();
    auto gainSection = getLocalBounds().removeFromLeft(getLocalBounds().getWidth()/2);

    //Gain Section
    gainLabel.setBounds(gainSection.removeFromTop(30));
    gainSlider.setBounds(gainSection.reduced(10));

    //Noise Gate Section
    noiseGateLabel.setBounds(noiseGateSection.removeFromTop(30));
    auto noiseGateDialsArea = noiseGateSection.reduced(10);
    int dialWidth = noiseGateDialsArea.getWidth() / 3;

    //Not sure if this is the best way to do it. It's really annoying to adjust
    noiseGateThresholdSlider.setBounds(noiseGateDialsArea.removeFromLeft(dialWidth).reduced(5));
    noiseGateAttackSlider.setBounds(noiseGateDialsArea.removeFromLeft(dialWidth).reduced(5));
    noiseGateReleaseSlider.setBounds(noiseGateDialsArea.reduced(5));
}
//This function checks to see if any slider's value has changed
void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    //This part checks the gain slider AUSTIN HILLS
    if (slider == &gainSlider)
    {
        processorRef.gainDB = (float)gainSlider.getValue();
    }
    //This part checks the noise gate threshold slider AUSTIN HILLS
    if (slider == &noiseGateThresholdSlider)
    {
        processorRef.gateThresholdDb = (float)noiseGateThresholdSlider.getValue();
    }
    //This part checks the noise gate attack slider AUSTIN HILLS
    if (slider == &noiseGateAttackSlider)
    {
        processorRef.gateAttack = (float)noiseGateAttackSlider.getValue();
    }
    //This part checks the noise gate release slider AUSTIN HILLS
    if (slider == &noiseGateReleaseSlider)
    {
        processorRef.gateRelease = (float)noiseGateReleaseSlider.getValue();
    }
}