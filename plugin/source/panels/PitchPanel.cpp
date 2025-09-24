#include "Pitchblade/panels/PitchPanel.h"

PitchPanel::PitchPanel(AudioPluginAudioProcessor& proc)
    : processor(proc)
{
    startTimerHz(8);    // Update 4x/second
}

void PitchPanel::resized()
{
    auto area = getLocalBounds().reduced(10);
}

void PitchPanel::paint(juce::Graphics& g)
{
    g.drawText(processor.getPitchDetector().getCurrentNoteName(), 0, 50, getWidth(), 50, juce::Justification::centred);
}

void PitchPanel::timerCallback() {
    repaint();
}