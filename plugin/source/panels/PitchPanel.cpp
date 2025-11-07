#include "Pitchblade/panels/PitchPanel.h"
#include "Pitchblade/ui/ColorPalette.h"

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
    g.setFont(50.0f); 
    g.setColour(Colors::accent);
    g.drawText(processor.getPitchCorrector().getCurrentNoteName(), 0, 50, getWidth(), 50, juce::Justification::centred);
}

void PitchPanel::timerCallback() {
    repaint();
}

// XML serialization for saving/loading - reyna
std::unique_ptr<juce::XmlElement> PitchNode::toXml() const {
    auto xml = std::make_unique<juce::XmlElement>("PitchNode");
    xml->setAttribute("name", effectName);
    xml->setAttribute("PitchAmount", (float)getNodeState().getProperty("PITCH_AMOUNT", 0.0f));
    xml->setAttribute("PitchCorrection", (int)getNodeState().getProperty("PITCH_CORRECTION", 0));
    xml->setAttribute("PitchMix", (float)getNodeState().getProperty("PITCH_MIX", 100.0f));
    return xml;
}

void PitchNode::loadFromXml(const juce::XmlElement& xml) {
    auto& s = getMutableNodeState();
    s.setProperty("PITCH_AMOUNT", (float)xml.getDoubleAttribute("PitchAmount", 0.0f), nullptr);
    s.setProperty("PITCH_CORRECTION", (int)xml.getIntAttribute("PitchCorrection", 0), nullptr);
    s.setProperty("PITCH_MIX", (float)xml.getDoubleAttribute("PitchMix", 100.0f), nullptr);
}
