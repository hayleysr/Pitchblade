//Austin Hills

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/PluginProcessor.h"

//The settings panel class inherits from component rather than effectpanel, since there are too many differences in application
class SettingsPanel : public juce::Component {
private:
    //Needs a reference to the processor to get the APVTS
    AudioPluginAudioProcessor& processor;

    juce::Label framerateLabel;
    //Used a combobox to list specific user choices
    juce::ComboBox framerateDropDown;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> framerateAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel)
public:
    //Constructor
    SettingsPanel(AudioPluginAudioProcessor& p);

    //Destructor
    ~SettingsPanel() override;

    void paint(juce::Graphics& g) override;

    void resized() override;
};