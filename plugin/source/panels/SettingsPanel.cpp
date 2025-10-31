//Austin Hills

#include "Pitchblade/panels/SettingsPanel.h"

SettingsPanel::SettingsPanel(){
    //Add buttons and such and make them visible
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
}