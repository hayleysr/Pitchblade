// austin

#include "Pitchblade/panels/NoiseGatePanel.h"
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "BinaryData.h"

//noise gate panel display
NoiseGatePanel::NoiseGatePanel(AudioPluginAudioProcessor& proc, juce::ValueTree& state) : processor(proc), localState(state) {
    // Label
    noiseGateLabel.setText("Noise Gate", juce::dontSendNotification);
    addAndMakeVisible(noiseGateLabel);

    // Threshold slider
    thresholdSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
      //Added these two to make them more nice looking and obvious for what they are - Austin
    thresholdSlider.setNumDecimalPlacesToDisplay(1);
    thresholdSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(thresholdSlider);
    
    // Threshold Label - Austin   
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(thresholdLabel);

    ////////////////////

    // Attack slider
    attackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    attackSlider.setNumDecimalPlacesToDisplay(1);
    attackSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(attackSlider);

    // Attack Label - Austin
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackLabel);

    ////////////////////

    // Release slider
    releaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    releaseSlider.setNumDecimalPlacesToDisplay(1);
    releaseSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(releaseSlider);

    // Release Label - Austin 
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(releaseLabel);

    ///////////////////
    //reynas changes - adding value tree functionality
    const float startThresholdDb = (float)localState.getProperty("GateThreshold", 0.0f);      // get starting gain from local state, if none, default to 0.0f
    thresholdSlider.setRange(-80.0, -48.0, 0.1);                                        // set slider range
    thresholdSlider.setValue(startThresholdDb, juce::dontSendNotification);                      // set slider to match starting gain
    thresholdSlider.onValueChange = [this]() {
        localState.setProperty("GateThreshold", (float)thresholdSlider.getValue(), nullptr);
        };

    const float startAttackMs = (float)localState.getProperty("GateAttack", 25.0f);
    attackSlider.setRange(1.0f, 200.0f, 1.0f);
    attackSlider.setValue(startAttackMs, juce::dontSendNotification);
    attackSlider.onValueChange = [this]() {
        localState.setProperty("GateAttack", (float)attackSlider.getValue(), nullptr);
        };

    const float startReleaseMs = (float)localState.getProperty("GateRelease", 100.0f);
    releaseSlider.setRange(10.0f, 1000.0f, 1.0f);
    releaseSlider.setValue(startReleaseMs, juce::dontSendNotification);
    releaseSlider.onValueChange = [this]() {
        localState.setProperty("GateRelease", (float)releaseSlider.getValue(), nullptr);
        };
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
void NoiseGatePanel::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);
    //g.setColour(Colors::accent);
    g.drawRect(getLocalBounds(), 2);
}

// destructor   - reyna
NoiseGatePanel::~NoiseGatePanel() {
    if (localState.isValid())
        localState.removeListener(this);
}

// value tree listener callback - reyna
void NoiseGatePanel::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) {
    if (tree == localState) {
        if (property == juce::Identifier("GateThreshold"))
            thresholdSlider.setValue((float)tree.getProperty("GateThreshold"), juce::dontSendNotification);
        else if (property == juce::Identifier("GateAttack"))
            attackSlider.setValue((float)tree.getProperty("GateAttack"), juce::dontSendNotification);
        else if (property == juce::Identifier("GateRelease"))
            releaseSlider.setValue((float)tree.getProperty("GateRelease"), juce::dontSendNotification);
    }
}
