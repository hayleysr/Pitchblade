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
    chooser->launchAsync(juce::FileBrowserComponent::saveMode |
        juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& c)
        {
            auto result = c.getResult();
            if (result != juce::File{}) {
                processor.savePresetToFile(result);
                juce::Logger::outputDebugString("Preset saved: " + result.getFullPathName());
            }
            chooser.reset();
        });
}

void PresetsPanel::handleLoadPreset() {
    auto initialDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Pitchblade/Presets");
    initialDir.createDirectory();

    chooser = std::make_unique<juce::FileChooser>("Load Preset", initialDir, "*.xml");

    chooser->launchAsync(juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& c)
        {
            auto result = c.getResult();
            if (result.existsAsFile()) {
                processor.loadPresetFromFile(result);
                juce::Logger::outputDebugString("Preset loaded: " + result.getFullPathName());
            }
            chooser.reset();
        });
}

void PresetsPanel::handleDefaultPreset() {
	// reset to default preset defined in AudioPluginAudioProcessor
    processor.loadDefaultPreset("Default");
    juce::Logger::outputDebugString("Loaded default preset (All Effects)");
}
