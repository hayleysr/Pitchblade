// reyna
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/panels/EffectNode.h"

//panel that shows audio visuals
class VisualizerPanel : public juce::Component
{
public:
    VisualizerPanel();
    void resized() override;
    void paint(juce::Graphics& g) override;

    void showVisualizer(int index);
    void refreshTabs();

private:
    juce::TabbedComponent tabs{ juce::TabbedButtonBar::TabsAtTop };
};