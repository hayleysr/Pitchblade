// Written by Austin Hills
// Refactored to use local ValueTree state

#include "Pitchblade/panels/DeEsserPanel.h"
#include "Pitchblade/ui/ColorPalette.h"

DeEsserPanel::DeEsserPanel(AudioPluginAudioProcessor& proc, juce::ValueTree& state) : processor(proc), localState(state)
{
    // Main Label
    deEsserLabel.setText("De-Esser", juce::dontSendNotification);
    addAndMakeVisible(deEsserLabel);

    // Threshold Slider
    thresholdSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    thresholdSlider.setNumDecimalPlacesToDisplay(1);
    thresholdSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(thresholdSlider);

    // Threshold Label
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(thresholdLabel);

    // Ratio Slider
    ratioSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    ratioSlider.setNumDecimalPlacesToDisplay(1);
    ratioSlider.setTextValueSuffix(" : 1");
    addAndMakeVisible(ratioSlider);

    // Ratio Label
    ratioLabel.setText("Ratio", juce::dontSendNotification);
    ratioLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(ratioLabel);

    // Attack Slider
    attackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    attackSlider.setNumDecimalPlacesToDisplay(1);
    attackSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(attackSlider);

    // Attack Label
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackLabel);

    // Release Slider
    releaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    releaseSlider.setNumDecimalPlacesToDisplay(1);
    releaseSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(releaseSlider);
    
    // Release Label
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(releaseLabel);
    
    // Frequency Slider
    frequencySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    frequencySlider.setNumDecimalPlacesToDisplay(0);
    frequencySlider.setTextValueSuffix(" Hz");
    addAndMakeVisible(frequencySlider);
    
    // Frequency Label
    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    frequencyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(frequencyLabel);

    ///////////////////
    // Link sliders to local state properties
    const float startThresholdDb = (float)localState.getProperty("DeEsserThreshold", 0.0f);
    thresholdSlider.setRange(-60.0, 0.0, 0.1);
    thresholdSlider.setValue(startThresholdDb, juce::dontSendNotification);
    thresholdSlider.onValueChange = [this]() {
        localState.setProperty("DeEsserThreshold", (float)thresholdSlider.getValue(), nullptr);
        };

    const float startRatio = (float)localState.getProperty("DeEsserRatio", 4.0f);
    ratioSlider.setRange(1.0f, 20.0f, 0.1f);
    ratioSlider.setValue(startRatio, juce::dontSendNotification);
    ratioSlider.onValueChange = [this]() {
        localState.setProperty("DeEsserRatio", (float)ratioSlider.getValue(), nullptr);
        };
    
    const float startAttackMs = (float)localState.getProperty("DeEsserAttack", 5.0f);
    attackSlider.setRange(10.0f, 200.0f, 0.1f);
    attackSlider.setValue(startAttackMs, juce::dontSendNotification);
    attackSlider.onValueChange = [this]() {
        localState.setProperty("DeEsserAttack", (float)attackSlider.getValue(), nullptr);
        };
    
    const float startReleaseMs = (float)localState.getProperty("DeEsserRelease", 5.0f);
    releaseSlider.setRange(10.0f, 1000.0f, 0.1f);
    releaseSlider.setValue(startReleaseMs, juce::dontSendNotification);
    releaseSlider.onValueChange = [this]() {
        localState.setProperty("DeEsserRelease", (float)releaseSlider.getValue(), nullptr);
        };

    const float startFrequency = (float)localState.getProperty("DeEsserFrequency", 6000.0f);
    frequencySlider.setRange(2000.0, 12000.0, 10.0);
    frequencySlider.setValue(startFrequency, juce::dontSendNotification);
    frequencySlider.onValueChange = [this]() {
        localState.setProperty("DeEsserFrequency", (float)frequencySlider.getValue(), nullptr);
        };
    
    // Add this panel as a listener to the local state
    localState.addListener(this);
}

void DeEsserPanel::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);
    g.drawRect(getLocalBounds(), 2);
}

DeEsserPanel::~DeEsserPanel()
{
    if (localState.isValid())
        localState.removeListener(this);
}

void DeEsserPanel::resized()
{
    auto area = getLocalBounds();

    // Title label at the top
    deEsserLabel.setBounds(area.removeFromTop(30));

    auto dials = area.reduced(10);
    int dialWidth = dials.getWidth() / 5;

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

// value tree listener callback
void DeEsserPanel::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (tree == localState)
    {
        if (property == juce::Identifier("DeEsserThreshold"))
            thresholdSlider.setValue((float)tree.getProperty("DeEsserThreshold"), juce::dontSendNotification);
        else if (property == juce::Identifier("DeEsserRatio"))
            ratioSlider.setValue((float)tree.getProperty("DeEsserRatio"), juce::dontSendNotification);
        else if (property == juce::Identifier("DeEsserAttack"))
            attackSlider.setValue((float)tree.getProperty("DeEsserAttack"), juce::dontSendNotification);
        else if (property == juce::Identifier("DeEsserRelease"))
            releaseSlider.setValue((float)tree.getProperty("DeEsserRelease"), juce::dontSendNotification);
        else if (property == juce::Identifier("DeEsserFrequency"))
            frequencySlider.setValue((float)tree.getProperty("DeEsserFrequency"), juce::dontSendNotification);
    }
}