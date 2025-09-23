// reyna macabebe
// austin


#include "Pitchblade/panels/GainPanel.h"
//gain panel display
GainPanel::GainPanel(AudioPluginAudioProcessor& proc) : processor(proc)
{
    // //add slider and set
    // addAndMakeVisible(gainSlider);
    // gainSlider.setRange(-48.0, 48.0);
    // gainSlider.addListener(this);

    // Gain Label - Austin
    gainLabel.setText("Gain", juce::dontSendNotification);
    addAndMakeVisible(gainLabel);

    // Gain slider - Austin
    gainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //Set the isReadOnly flag to false to allow user to edit - Austin
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    gainSlider.setRange(-48,48);
    gainSlider.setValue(processor.gainDB);
    //Added these two to make them more nice looking and obvious for what they are - Austin
    gainSlider.setNumDecimalPlacesToDisplay(1);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.addListener(this);
    addAndMakeVisible(gainSlider);

}

void GainPanel::resized()
{
    // Label at top - Austin
    gainLabel.setBounds(getLocalBounds().removeFromTop(30));

    //Slider
    gainSlider.setBounds(getLocalBounds().reduced(10));
}
//updates gainDB in AudioPluginAudioProcessor , when slider changes value changes
void GainPanel::sliderValueChanged(juce::Slider* s)
{
    if (s == &gainSlider)
        processor.gainDB = (float)gainSlider.getValue();
}
