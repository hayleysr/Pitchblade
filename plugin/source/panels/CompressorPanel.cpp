// Written by Austin Hills
// Redone to use APVTS attachments

#include "Pitchblade/panels/CompressorPanel.h"
#include "Pitchblade/ui/ColorPalette.h"

//Set up of UI components
CompressorPanel::CompressorPanel(AudioPluginAudioProcessor& proc) : processor(proc)
{
    // Label
    compressorLabel.setText("Compressor", juce::dontSendNotification);
    addAndMakeVisible(compressorLabel);

    // Mode Button
    modeButton.setClickingTogglesState(true);
    addAndMakeVisible(modeButton);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.apvts, "COMP_LIMITER_MODE", modeButton);

    //This updates the UI when the button is clicked
    modeButton.onClick = [this](){
        updateSliderVisibility();
    };

    // Threshold slider
    thresholdSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    thresholdSlider.setNumDecimalPlacesToDisplay(1);
    thresholdSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(thresholdSlider);

    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "COMP_THRESHOLD", thresholdSlider);

    // Threshold Label - Austin
    addAndMakeVisible(thresholdLabel);
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.setJustificationType(juce::Justification::centred);

    // Ratio slider
    ratioSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    ratioSlider.setNumDecimalPlacesToDisplay(1);
    ratioSlider.setTextValueSuffix(" : 1");
    addAndMakeVisible(ratioSlider);

    ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "COMP_RATIO", ratioSlider);

    // Ratio Label - Austin
    addAndMakeVisible(ratioLabel);
    ratioLabel.setText("Ratio", juce::dontSendNotification);
    ratioLabel.setJustificationType(juce::Justification::centred);

    // Attack slider
    attackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    attackSlider.setNumDecimalPlacesToDisplay(1);
    attackSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(attackSlider);

    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "COMP_ATTACK", attackSlider);

    // Attack Label - Austin
    addAndMakeVisible(attackLabel);
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);

    // Release slider
    releaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    releaseSlider.setNumDecimalPlacesToDisplay(1);
    releaseSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(releaseSlider);

    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "COMP_RELEASE", releaseSlider);

    // Release Label - Austin
    addAndMakeVisible(releaseLabel);
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);

    //Initializing visibility of sliders
    updateSliderVisibility();
};

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

void CompressorPanel::updateSliderVisibility()
{
    //Get the current state from the toggle state
    bool isLimiter = modeButton.getToggleState();

    if(isLimiter==0){
        modeButton.setButtonText("Limiter Mode");
    }else{
        modeButton.setButtonText("Compressor Mode");
    }

    //Show or hide controls based on the mode
    ratioSlider.setVisible(isLimiter);
    ratioLabel.setVisible(isLimiter);
    attackSlider.setVisible(isLimiter);
    attackLabel.setVisible(isLimiter);
}

void CompressorPanel::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);
    //g.setColour(Colors::accent);
    g.drawRect(getLocalBounds(), 2);
}