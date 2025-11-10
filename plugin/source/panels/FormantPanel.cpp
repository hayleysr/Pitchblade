//hudas code
#include "Pitchblade/panels/FormantPanel.h"
#include "Pitchblade/ui/ColorPalette.h"

FormantPanel::FormantPanel(AudioPluginAudioProcessor& proc)
    : processor(proc)
{
    // Labels + sliders
    formantLabel.setText("Formant", juce::dontSendNotification);
    addAndMakeVisible(formantLabel);

    formantSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    formantSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    formantSlider.setRange(-50.0, 50.0, 0.1);
    formantSlider.setSkewFactorFromMidPoint(1.0);
    addAndMakeVisible(formantSlider);

    mixLabel.setText("Dry/Wet", juce::dontSendNotification);
    addAndMakeVisible(mixLabel);

    mixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    mixSlider.setRange(0.0, 1.0, 0.001);
    addAndMakeVisible(mixSlider);

    // Attachments
    formantAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, PARAM_FORMANT_SHIFT, formantSlider);
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, PARAM_FORMANT_MIX, mixSlider);

}

void FormantPanel::resized()
{
    auto r = getLocalBounds().reduced(10);
    // Dynamically size rows to reduce empty space below controls
    const int gap = 10;
    int rowHeight = juce::jmax(40, (r.getHeight() - gap) / 2);
    auto row1 = r.removeFromTop(rowHeight);
    formantLabel .setBounds(row1.removeFromLeft(90));
    formantSlider.setBounds(row1);

    r.removeFromTop(gap);
    auto row2 = r.removeFromTop(rowHeight);
    mixLabel.setBounds(row2.removeFromLeft(90));
    mixSlider.setBounds(row2);
}

void FormantPanel::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);
}


// XML serialization for saving/loading - reyna
std::unique_ptr<juce::XmlElement> FormantNode::toXml() const {
    auto xml = std::make_unique<juce::XmlElement>("FormantNode");
    xml->setAttribute("name", effectName);
    xml->setAttribute("FormantShift", (float)getNodeState().getProperty("FORMANT_SHIFT", 0.0f));
    xml->setAttribute("Mix", (float)getNodeState().getProperty("FORMANT_MIX", 100.0f));
    return xml;
}

void FormantNode::loadFromXml(const juce::XmlElement& xml) {
    auto& s = getMutableNodeState();
    s.setProperty("FORMANT_SHIFT", (float)xml.getDoubleAttribute("FormantShift", 0.0f), nullptr);
    s.setProperty("FORMANT_MIX", (float)xml.getDoubleAttribute("Mix", 100.0f), nullptr);
}
