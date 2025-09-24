// austin

#include "Pitchblade/panels/NoiseGatePanel.h"
//noise gate panel display
NoiseGatePanel::NoiseGatePanel(AudioPluginAudioProcessor& proc) : processor(proc)
{
    // Label
    noiseGateLabel.setText("Noise Gate", juce::dontSendNotification);
    addAndMakeVisible(noiseGateLabel);

    // Threshold slider
    thresholdSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    thresholdSlider.setRange(-60.0, 0.0);
    thresholdSlider.setValue(processor.gateThresholdDb);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    thresholdSlider.setNumDecimalPlacesToDisplay(1);
    thresholdSlider.setTextValueSuffix(" dB");
    thresholdSlider.addListener(this);
    addAndMakeVisible(thresholdSlider);

    // Threshold Label - Austin
    addAndMakeVisible(thresholdLabel);
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.setJustificationType(juce::Justification::centred);

    // Attack slider
    attackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    attackSlider.setRange(0.1, 300.0);
    attackSlider.setValue(processor.gateAttack);
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
    releaseSlider.setValue(processor.gateRelease);
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

void NoiseGatePanel::resized()
{
    auto area = getLocalBounds();

    // Label at top
    noiseGateLabel.setBounds(area.removeFromTop(30));

    // Divide remaining width for 3 dials
    auto dials = area.reduced(10);
    int dialWidth = dials.getWidth() / 3;

    // Create areas for each slider and label

    auto thresholdArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto attackArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto releaseArea = dials.reduced(5);
    
    //Positioning threshold label and slider
    thresholdLabel.setBounds(thresholdArea.removeFromTop(20));
    thresholdSlider.setBounds(thresholdArea);

    //Positioning attack label and slider
    attackLabel.setBounds(attackArea.removeFromTop(20));
    attackSlider.setBounds(attackArea);

    //Positioning release label and slider
    releaseLabel.setBounds(releaseArea.removeFromTop(20));
    releaseSlider.setBounds(releaseArea);
}
//updates gainDB in AudioPluginAudioProcessor , when slider changes value changes
void NoiseGatePanel::sliderValueChanged(juce::Slider* s)
{
    if (s == &thresholdSlider)
        processor.gateThresholdDb = (float)thresholdSlider.getValue();
    else if (s == &attackSlider)
        processor.gateAttack = (float)attackSlider.getValue();
    else if (s == &releaseSlider)
        processor.gateRelease = (float)releaseSlider.getValue();
}
