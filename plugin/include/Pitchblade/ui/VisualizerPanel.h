#pragma once
#include <JuceHeader.h>


//panel that shows audio visuals
class VisualizerPanel : public juce::Component
{
public:
    VisualizerPanel();
    void paint(juce::Graphics& g) override;
};