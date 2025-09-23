// reyna macabebe
// austin


#include "Pitchblade/panels/GainPanel.h"
//gain panel display
GainPanel::GainPanel(AudioPluginAudioProcessor& proc) : processor(proc)
{
    //add slider and set
    addAndMakeVisible(gainSlider);
    gainSlider.setRange(-48.0, 48.0);
    gainSlider.addListener(this);
}

void GainPanel::resized()
{
    gainSlider.setBounds(getLocalBounds().reduced(10));
}
//updates gainDB in AudioPluginAudioProcessor , when slider changes value changes
void GainPanel::sliderValueChanged(juce::Slider* s)
{
    if (s == &gainSlider)
        processor.gainDB = (float)gainSlider.getValue();
}
