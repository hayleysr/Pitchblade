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
    auto area = getLocalBounds();

    //Title label at the top
    compressorLabel.setBounds(area.removeFromTop(30));

    modeButton.setBounds(area.removeFromTop(30).reduced(10,5));

    auto dials = area.reduced(10);
    int dialWidth = dials.getWidth()/4;

    auto thresholdArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto ratioArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto attackArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto releaseArea = dials.reduced(5);

    //Positioning threshold label and slider
    thresholdLabel.setBounds(thresholdArea.removeFromTop(20));
    thresholdSlider.setBounds(thresholdArea);

    //Positioning ratio label and slider
    ratioLabel.setBounds(ratioArea.removeFromTop(20));
    ratioSlider.setBounds(ratioArea);

    //Positioning attack label and slider
    attackLabel.setBounds(attackArea.removeFromTop(20));
    attackSlider.setBounds(attackArea);

    //Positioning release label and slider
    releaseLabel.setBounds(releaseArea.removeFromTop(20));
    releaseSlider.setBounds(releaseArea);
}

void CompressorPanel::sliderValueChanged(juce::Slider* s)
{
    if (s == &thresholdSlider)
        processor.compressorThresholdDb = (float)thresholdSlider.getValue();
    else if (s == &ratioSlider)
        processor.compressorRatio = (float)ratioSlider.getValue();
    else if (s == &attackSlider)
        processor.compressorAttack = (float)attackSlider.getValue();
    else if (s == &releaseSlider)
        processor.compressorRelease = (float)releaseSlider.getValue();
}

void CompressorPanel::buttonClicked(juce::Button* b)
{
    if(b == &modeButton){
        processor.isLimiterMode = b->getToggleState();
        if(processor.isLimiterMode == 0){
            b->setButtonText("Simple Mode");
        }else{
            b->setButtonText("Limiter Mode");
        }
        updateSliderVisibility();
    }
}

void CompressorPanel::updateSliderVisibility()
{
    ratioSlider.setVisible(!processor.isLimiterMode);
    ratioLabel.setVisible(!processor.isLimiterMode);
    attackSlider.setVisible(!processor.isLimiterMode);
    attackLabel.setVisible(!processor.isLimiterMode);
}

void NoiseGatePanel::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);
    //g.setColour(Colors::accent);
    g.drawRect(getLocalBounds(), 2);
}