// reyna
#include "Pitchblade/panels/PresetsPanel.h"

// preset panel shows "save preset", "load preset", "default preset" buttons
// allows user to save/load presets to/from xml files
// default preset resets all parameters to default values 

PresetsPanel::PresetsPanel(AudioPluginAudioProcessor& proc) : processor(proc) {
    addAndMakeVisible(saveButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(defaultButton);

    saveButton.onClick = [this]() { handleSavePreset(); };
    loadButton.onClick = [this]() { handleLoadPreset(); };
    defaultButton.onClick = [this]() { handleDefaultPreset(); };

    // preset change confirmation message
    addAndMakeVisible(statusLabel);
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, Colors::buttonText);
    statusLabel.setText("", juce::dontSendNotification);

    statusLabel.setText("Preset loaded successfully!", juce::dontSendNotification);
    juce::Timer::callAfterDelay(2000, [this]() {
        statusLabel.setText("", juce::dontSendNotification); 
        });

}

void PresetsPanel::paint(juce::Graphics& g) {
    g.fillAll(Colors::background);
    g.setColour(Colors::accent);
    g.setFont(24.0f);
    g.drawText("Preset Manager", getLocalBounds().removeFromTop(50), juce::Justification::centred, true);
}

void PresetsPanel::resized() {
    auto area = getLocalBounds().reduced(40);
    const int buttonH = 40;

    saveButton.setBounds(area.removeFromTop(buttonH).reduced(0, 10));
    loadButton.setBounds(area.removeFromTop(buttonH).reduced(0, 10));
    defaultButton.setBounds(area.removeFromTop(buttonH).reduced(0, 10));

    // Status label
    area.removeFromTop(10);
    statusLabel.setBounds(area.removeFromTop(30));
}

/////////////////////////////////
// preset handlers

void PresetsPanel::handleSavePreset() {
	// makes sure preset directory exists
    auto initialDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Pitchblade/Presets");
    initialDir.createDirectory();

	// open file chooser to save preset
    chooser = std::make_unique<juce::FileChooser>("Save Preset", initialDir, "*.xml");

	// launch save dialog
    chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,[this](const juce::FileChooser& c) {
            auto result = c.getResult();
            if (result != juce::File{}) {
                processor.savePresetToFile(result);
                // update label
                statusLabel.setText("Preset saved!", juce::dontSendNotification);
                juce::MessageManager::callAsync([this]() {
                    if (onPresetActionFinished) onPresetActionFinished();
                    });
            } else {
                statusLabel.setText("Save canceled", juce::dontSendNotification);
            }
            chooser.reset();
        });
	if (onPresetActionFinished) onPresetActionFinished();   // callback after preset action
}

void PresetsPanel::handleLoadPreset() {
    auto initialDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Pitchblade/Presets");
    initialDir.createDirectory();

    chooser = std::make_unique<juce::FileChooser>("Load Preset", initialDir, "*.xml");

    chooser->launchAsync(juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& c) {
            auto result = c.getResult();
            if (result.existsAsFile()) {
                processor.loadPresetFromFile(result);
                // label 
                statusLabel.setText("Preset loaded!", juce::dontSendNotification);
                juce::MessageManager::callAsync([this]() {
                    if (onPresetActionFinished) onPresetActionFinished();
                    });
            } else {
                statusLabel.setText("Load canceled", juce::dontSendNotification);
            }
            chooser.reset();
        });
    if (onPresetActionFinished) onPresetActionFinished();
}

void PresetsPanel::handleDefaultPreset() {
	// reset to default preset defined in AudioPluginAudioProcessor
    processor.loadDefaultPreset("Default");
    statusLabel.setText("Default preset loaded", juce::dontSendNotification);

    juce::MessageManager::callAsync([this]() {
        if (onPresetActionFinished) onPresetActionFinished();
        });
}
