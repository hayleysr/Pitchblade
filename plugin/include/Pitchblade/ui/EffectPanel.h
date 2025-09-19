#pragma once
#include <JuceHeader.h>


//tabs for each effect
class EffectPanel : public juce::Component
{
public:
    EffectPanel();

    void resized() override;
    void showEffect(int index);

private:
    //top tab buttons
    juce::TabbedComponent tabs{ juce::TabbedButtonBar::TabsAtTop };
};