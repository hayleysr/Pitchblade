// reyna macabebe
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/ui/EffectRegistry.h"


//tabs for each effect
class EffectPanel : public juce::Component
{
public:
    explicit EffectPanel(AudioPluginAudioProcessor& proc);

    void resized() override;
    void showEffect(int index);

private:
    //top tab buttons
    juce::TabbedComponent tabs{ juce::TabbedButtonBar::TabsAtTop };
};