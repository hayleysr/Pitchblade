//reyna
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/PluginProcessor.h"

//panel that shows preset management
// load presets, save presets, default presets

class PresetsPanel : public juce::Component {
public:
    explicit PresetsPanel(AudioPluginAudioProcessor& proc);
    ~PresetsPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AudioPluginAudioProcessor& processor;

    juce::TextButton saveButton{ "Save Preset" };
    juce::TextButton loadButton{ "Load Preset" };
    juce::TextButton defaultButton{ "Use Default Preset" };

	// file chooser for loading/saving presets
    std::unique_ptr<juce::FileChooser> chooser;

    void handleSavePreset();
    void handleLoadPreset();
    void handleDefaultPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetsPanel)
};