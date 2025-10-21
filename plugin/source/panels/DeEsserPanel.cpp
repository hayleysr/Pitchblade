// Written by Austin Hills

#include "Pitchblade/panels/DeEsserPanel.h"
#include "Pitchblade/ui/ColorPalette.h"

DeEsserPanel::DeEsserPanel(AudioPluginAudioProcessor& proc) : processor(proc)
{
    // Main Label
    deEsserLabel.setText("De-Esser", juce::dontSendNotification);

    // Threshold Slider & Attachment
    thresholdSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    thresholdSlider.setNumDecimalPlacesToDisplay(1);
    thresholdSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(thresholdSlider);
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "DEESSER_THRESHOLD", thresholdSlider);

    // Threshold Label
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(thresholdLabel);

    // Ratio Slider & Attachment
    ratioSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    ratioSlider.setNumDecimalPlacesToDisplay(1);
    ratioSlider.setTextValueSuffix(" : 1");
    addAndMakeVisible(ratioSlider);
    ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "DEESSER_RATIO", ratioSlider);

    // Ratio Label
    ratioLabel.setText("Ratio", juce::dontSendNotification);
    ratioLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(ratioLabel);

    // Attack Slider & Attachment
    attackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    attackSlider.setNumDecimalPlacesToDisplay(1);
    attackSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(attackSlider);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "DEESSER_ATTACK", attackSlider);

    // Attack Label
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackLabel);

    // Release Slider & Attachment
    releaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    releaseSlider.setNumDecimalPlacesToDisplay(1);
    releaseSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(releaseSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "DEESSER_RELEASE", releaseSlider);
    
    // Release Label
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(releaseLabel);
    
    // Frequency Slider & Attachment
    frequencySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    frequencySlider.setNumDecimalPlacesToDisplay(0);
    frequencySlider.setTextValueSuffix(" Hz");
    addAndMakeVisible(frequencySlider);
    frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "DEESSER_FREQUENCY", frequencySlider);
    
    // Frequency Label
    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    frequencyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(frequencyLabel);
}

void DeEsserPanel::resized()
{
    auto area = getLocalBounds();

    // Title label at the top
    deEsserLabel.setBounds(area.removeFromTop(30));

    auto dials = area.reduced(10);
    int dialWidth = dials.getWidth() / 5; // We have 5 dials now

    auto frequencyArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto thresholdArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto ratioArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto attackArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto releaseArea = dials.reduced(5);

    // Positioning frequency label and slider
    frequencyLabel.setBounds(frequencyArea.removeFromTop(20));
    frequencySlider.setBounds(frequencyArea);
    
    // Positioning threshold label and slider
    thresholdLabel.setBounds(thresholdArea.removeFromTop(20));
    thresholdSlider.setBounds(thresholdArea);

    // Positioning ratio label and slider
    ratioLabel.setBounds(ratioArea.removeFromTop(20));
    ratioSlider.setBounds(ratioArea);

    // Positioning attack label and slider
    attackLabel.setBounds(attackArea.removeFromTop(20));
    attackSlider.setBounds(attackArea);

    // Positioning release label and slider
    releaseLabel.setBounds(releaseArea.removeFromTop(20));
    releaseSlider.setBounds(releaseArea);
}

void DeEsserPanel::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);
    g.drawRect(getLocalBounds(), 2);
}