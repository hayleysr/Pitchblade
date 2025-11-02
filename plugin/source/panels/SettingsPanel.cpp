//Austin Hills

#include "Pitchblade/panels/SettingsPanel.h"

SettingsPanel::SettingsPanel(AudioPluginAudioProcessor& p) : processor(p) {
    //Framerate label
    framerateLabel.setText("Graph FPS:", juce::dontSendNotification);
    framerateLabel.setJustificationType(juce::Justification::centredLeft);
    framerateLabel.setColour(juce::Label::textColourId,Colors::buttonText);
    addAndMakeVisible(framerateLabel);

    //Framerate Menu
    framerateDropDown.addItemList(juce::StringArray{"5 FPS", "15 FPS", "30 FPS", "60 FPS"},1);
    addAndMakeVisible(framerateDropDown);

    //Attach menu to parameter
    framerateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.apvts, "GLOBAL_FRAMERATE", framerateDropDown);
}

SettingsPanel::~SettingsPanel(){}

void SettingsPanel::paint(juce::Graphics& g){
    g.fillAll(Colors::background);

    //Simple placeholder text
    g.setColour(Colors::accent);
    g.setFont(20.0f);

    g.drawText("Settings Panel",getLocalBounds(),juce::Justification::centred,1);
}

void SettingsPanel::resized(){
    //Layout of UI elements
    auto area = getLocalBounds();

    area.removeFromTop(50);

    auto framerateArea = area.removeFromTop(40).reduced(20,0);

    framerateLabel.setBounds(framerateArea.removeFromLeft(framerateArea.getWidth()/3));

    framerateArea.removeFromLeft(10);

    framerateDropDown.setBounds(framerateArea);
}