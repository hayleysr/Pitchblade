//Austin Hills

#pragma once
#include <JuceHeader.h>
#include "Pitchblade/ui/ColorPalette.h"

//The settings panel class inherits from component rather than effectpanel, since there are too many differences in application
class SettingsPanel : public juce::Component {
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel)
public:
    //Constructor
    SettingsPanel();

    //Destructor
    ~SettingsPanel() override;

    void paint(juce::Graphics& g) override;

    void resized() override;
};