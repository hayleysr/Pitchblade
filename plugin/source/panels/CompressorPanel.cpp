// Written by Austin Hills
// Redone to use local ValueTree state instead of APVTS

#include "Pitchblade/panels/CompressorPanel.h"
#include <JuceHeader.h>
#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "BinaryData.h"

//Set up of UI components
CompressorPanel::CompressorPanel(AudioPluginAudioProcessor& proc, juce::ValueTree& state) : processor(proc), localState(state)
{
    // Label
    compressorLabel.setText("Compressor", juce::dontSendNotification);
    addAndMakeVisible(compressorLabel);

    // Mode Button
    //modeButton.setClickingTogglesState(true);
    //addAndMakeVisible(modeButton);

    static CustomLookAndFeel gSwitchLF;
    modeButton.setButtonText("Limiter Mode");
    modeButton.setClickingTogglesState(true);
    addAndMakeVisible(modeButton);

    // Set initial button state from local state
    const bool startLimiterMode = (bool)localState.getProperty("CompLimiterMode", false);
    modeButton.setToggleState(startLimiterMode, juce::dontSendNotification);
    
    // On click, update the local state property
    modeButton.onClick = [this]() {
        localState.setProperty("CompLimiterMode", modeButton.getToggleState(), nullptr);
    };

    // Threshold slider
    thresholdSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    thresholdSlider.setNumDecimalPlacesToDisplay(1);
    thresholdSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(thresholdSlider);

    // Threshold Label
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(thresholdLabel);
    
    // Ratio slider
    ratioSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    ratioSlider.setNumDecimalPlacesToDisplay(1);
    ratioSlider.setTextValueSuffix(" : 1");
    addAndMakeVisible(ratioSlider);
    
    // Ratio Label
    ratioLabel.setText("Ratio", juce::dontSendNotification);
    ratioLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(ratioLabel);

    // Attack slider
    attackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    attackSlider.setNumDecimalPlacesToDisplay(1);
    attackSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(attackSlider);
    
    // Attack Label
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackLabel);

    // Release slider
    releaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    releaseSlider.setNumDecimalPlacesToDisplay(1);
    releaseSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(releaseSlider);

    // Release Label
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(releaseLabel);

    ///////////////////
    // Link sliders to local state properties
    const float startThresholdDb = (float)localState.getProperty("CompThreshold", 0.0f);
    thresholdSlider.setRange(-100.0f, 0.0f, 0.1f);
    thresholdSlider.setValue(startThresholdDb, juce::dontSendNotification);
    thresholdSlider.onValueChange = [this]() {
        localState.setProperty("CompThreshold", (float)thresholdSlider.getValue(), nullptr);
        };

    const float startRatio = (float)localState.getProperty("CompRatio", 3.0f);
    ratioSlider.setRange(1.0f, 20.0f, 0.1f);
    ratioSlider.setValue(startRatio, juce::dontSendNotification);
    ratioSlider.onValueChange = [this]() {
        localState.setProperty("CompRatio", (float)ratioSlider.getValue(), nullptr);
        };
    
    const float startAttackMs = (float)localState.getProperty("CompAttack", 50.0f);
    attackSlider.setRange(1.0f, 200.0f, 1.0f);
    attackSlider.setValue(startAttackMs, juce::dontSendNotification);
    attackSlider.onValueChange = [this]() {
        localState.setProperty("CompAttack", (float)attackSlider.getValue(), nullptr);
        };
    
    const float startReleaseMs = (float)localState.getProperty("CompRelease", 250.0f);
    releaseSlider.setRange(10.0f, 1000.0f, 1.0f);
    releaseSlider.setValue(startReleaseMs, juce::dontSendNotification);
    releaseSlider.onValueChange = [this]() {
        localState.setProperty("CompRelease", (float)releaseSlider.getValue(), nullptr);
        };
    
    // Add this panel as a listener to the local state
    localState.addListener(this);

    //Initializing visibility of sliders
    updateSliderVisibility();
};

void CompressorPanel::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);
    g.drawRect(getLocalBounds(), 2);
}

CompressorPanel::~CompressorPanel()
{
    if (localState.isValid())
        localState.removeListener(this);
}

void CompressorPanel::resized()
{
    auto area = getLocalBounds();

    //Title label at the top
    compressorLabel.setBounds(area.removeFromTop(30));
    modeButton.setBounds(area.removeFromTop(30).reduced(10, 5));

    auto dials = area.reduced(10);
    int dialWidth = dials.getWidth() / 4;

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
    // Get the current state from the local state
    const bool isLimiter = (bool)localState.getProperty("CompLimiterMode", false);

    if (isLimiter) {
        modeButton.setButtonText("Compressor Mode");
    }
    else {
        modeButton.setButtonText("Limiter Mode");
    }

    // In limiter mode, hide the ratio and attack sliders
    const bool showCompressorControls = !isLimiter;
    ratioSlider.setVisible(showCompressorControls);
    ratioLabel.setVisible(showCompressorControls);
    attackSlider.setVisible(showCompressorControls);
    attackLabel.setVisible(showCompressorControls);
}

void CompressorPanel::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (tree == localState)
    {
        if (property == juce::Identifier("CompThreshold"))
            thresholdSlider.setValue((float)tree.getProperty("CompThreshold"), juce::dontSendNotification);
        else if (property == juce::Identifier("CompRatio"))
            ratioSlider.setValue((float)tree.getProperty("CompRatio"), juce::dontSendNotification);
        else if (property == juce::Identifier("CompAttack"))
            attackSlider.setValue((float)tree.getProperty("CompAttack"), juce::dontSendNotification);
        else if (property == juce::Identifier("CompRelease"))
            releaseSlider.setValue((float)tree.getProperty("CompRelease"), juce::dontSendNotification);
        else if (property == juce::Identifier("CompLimiterMode"))
        {
            modeButton.setToggleState((bool)tree.getProperty("CompLimiterMode"), juce::dontSendNotification);
            updateSliderVisibility();
        }
    }
}

CompressorVisualizer::~CompressorVisualizer(){
        //Stop listening
        if(localState.isValid()){
            localState.removeListener(this);
        }
    }

//Update the graph
void CompressorVisualizer::timerCallback(){
    float newDbLevel = compressorNode.getOutputLevelAtomic().load();

    //Push it to graph
    pushData(newDbLevel);

    //Call the graph visualizer's timerCallback
    RealTimeGraphVisualizer::timerCallback();
}

void CompressorVisualizer::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property){
    if(tree==localState && property == juce::Identifier("CompThreshold")){
        //Threshold slider changed, so update the line
        float newThreshold = (float)localState.getProperty("CompThreshold",0.0f);
        setThreshold(newThreshold,true);
    }
}

// XML serialization for saving/loading state - reyna 

std::unique_ptr<juce::XmlElement> CompressorNode::toXml() const {
    auto xml = std::make_unique<juce::XmlElement>("CompressorNode");
    xml->setAttribute("name", effectName);
    
    // Austin - Fixed the local state property names to camelCase
    xml->setAttribute("CompThreshold", (float)getNodeState().getProperty("CompThreshold", 0.0f));
    xml->setAttribute("CompRatio", (float)getNodeState().getProperty("CompRatio", 3.0f));
    xml->setAttribute("CompAttack", (float)getNodeState().getProperty("CompAttack", 50.0f));
    xml->setAttribute("CompRelease", (float)getNodeState().getProperty("CompRelease", 250.0f));
    xml->setAttribute("CompLimiterMode", (int)getNodeState().getProperty("CompLimiterMode", 0));
    
    return xml;
}

void CompressorNode::loadFromXml(const juce::XmlElement& xml) {
    auto& s = getMutableNodeState();

    // Austin - Fixed the local state property names to camelCase
    s.setProperty("CompThreshold", (float)xml.getDoubleAttribute("CompThreshold", 0.0f), nullptr);
    s.setProperty("CompRatio", (float)xml.getDoubleAttribute("CompRatio", 3.0f), nullptr);
    s.setProperty("CompAttack", (float)xml.getDoubleAttribute("CompAttack", 50.0f), nullptr);
    s.setProperty("CompRelease", (float)xml.getDoubleAttribute("CompRelease", 250.0f), nullptr);
    s.setProperty("CompLimiterMode", (int)xml.getIntAttribute("CompLimiterMode", 0), nullptr);
}
