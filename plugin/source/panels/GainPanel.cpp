// reyna
// austin

#include "Pitchblade/panels/GainPanel.h"
#include <JuceHeader.h>
#include "Pitchblade/ui/ColorPalette.h"
#include "BinaryData.h"


//gain panel display
GainPanel::GainPanel(AudioPluginAudioProcessor& proc, juce::ValueTree& state) : processor(proc), localState(state) {    // added valuetree and localstate - reyna

    // Gain Label - Austin
    gainLabel.setText("Gain", juce::dontSendNotification);
    addAndMakeVisible(gainLabel);

    // Gain slider - Austin
    gainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    
    //Added these two to make them more nice looking and obvious for what they are - Austin
    gainSlider.setNumDecimalPlacesToDisplay(1);
    gainSlider.setTextValueSuffix(" dB");
    //gainSlider.addListener(this);
    addAndMakeVisible(gainSlider);

	//reynas changes - adding value tree functionality
	const float startDb = (float)localState.getProperty("Gain", 0.0f);      // get starting gain from local state, if none, default to 0.0f
	gainSlider.setRange(-24.0, 24.0, 0.1);                                  // set slider range
	gainSlider.setValue(startDb, juce::dontSendNotification);               // set slider to match starting gain

	// update value tree on slider change
    gainSlider.onValueChange = [this]() { localState.setProperty("Gain", (float)gainSlider.getValue(), nullptr); };
	localState.addListener(this);   // listen to changes in local state
}

void GainPanel::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);
    //g.setColour(Colors::accent);
    g.drawRect(getLocalBounds(), 2);
}

void GainPanel::resized()
{
    // Label at top - Austin
    gainLabel.setBounds(getLocalBounds().removeFromTop(30));

    //Slider
    gainSlider.setBounds(getLocalBounds().reduced(10));
}
 
// destructor   - reyna
GainPanel::~GainPanel() {
    if (localState.isValid())
        localState.removeListener(this);
}

// value tree listener callback - reyna
void GainPanel::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) {
    if (tree == localState && property == juce::Identifier("Gain"))
        gainSlider.setValue((float)tree.getProperty("Gain"), juce::dontSendNotification);
}

