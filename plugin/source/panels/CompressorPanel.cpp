// Written by Austin Hills

#include "Pitchblade/panels/CompressorPanel.h"
#include "Pitchblade/ui/ColorPalette.h"

//Set up of UI components
CompressorPanel::CompressorPanel(AudioPluginAudioProcessor& proc) : processor(proc)
{
    // Label
    compressorLabel.setText("Compressor", juce::dontSendNotification);
    addAndMakeVisible(compressorLabel);

    // Mode Button
    // I have no idea if any of this works yet. I'll have to make sure that the components work before worrying about this. If it does, I'm going to try to save some variables so that ratio and attack aren't reset upon toggling this on and off.
    if(processor.IsLimiterMode == 0){
        modeButton.setButtonText("Compressor");
    }else{
        modeButton.setButtonText("Limiter");
    }
    modeButton.setClickingTogglesState(true);
    modeButton.setToggleState(processor.isLimiterMode, juce::dontSendNotification);
    modeButton.addListener(this);
    addAndMakeVisible(modeButton);

    // Threshold slider
    thresholdSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    thresholdSlider.setRange(-60.0, 0.0);
    thresholdSlider.setValue(processor.compressorThresholdDb);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    thresholdSlider.setNumDecimalPlacesToDisplay(1);
    thresholdSlider.setTextValueSuffix(" dB");
    thresholdSlider.addListener(this);
    addAndMakeVisible(thresholdSlider);

    // Threshold Label - Austin
    addAndMakeVisible(thresholdLabel);
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.setJustificationType(juce::Justification::centred);

    // Ratio slider
    ratioSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    ratioSlider.setRange(-60.0, 0.0);
    ratioSlider.setValue(processor.compressorRatioDb);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    ratioSlider.setNumDecimalPlacesToDisplay(1);
    ratioSlider.setTextValueSuffix(" : 1");
    ratioSlider.addListener(this);
    addAndMakeVisible(ratioSlider);

    // Ratio Label - Austin
    addAndMakeVisible(ratioLabel);
    ratioLabel.setText("Ratio", juce::dontSendNotification);
    ratioLabel.setJustificationType(juce::Justification::centred);

    // Attack slider
    attackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    attackSlider.setRange(0.1, 300.0);
    attackSlider.setValue(processor.compressorAttack);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    attackSlider.setNumDecimalPlacesToDisplay(1);
    attackSlider.setTextValueSuffix(" ms");
    attackSlider.addListener(this);
    addAndMakeVisible(attackSlider);

    // Attack Label - Austin
    addAndMakeVisible(attackLabel);
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);

    // Release slider
    releaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    releaseSlider.setRange(0.1, 300.0);
    releaseSlider.setValue(processor.compressorRelease);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    releaseSlider.setNumDecimalPlacesToDisplay(1);
    releaseSlider.setTextValueSuffix(" ms");
    releaseSlider.addListener(this);
    addAndMakeVisible(releaseSlider);

    // Release Label - Austin
    addAndMakeVisible(releaseLabel);
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);
}

void CompressorPanel::resized()
{

}