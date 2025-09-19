// reyna macabebe

#include "Pitchblade/ui/VisualizerPanel.h"


VisualizerPanel::VisualizerPanel() {}

void VisualizerPanel::paint(juce::Graphics& g)
{
    //placeholder
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.drawText("Visualizer Placeholder", getLocalBounds(), juce::Justification::centred);
}