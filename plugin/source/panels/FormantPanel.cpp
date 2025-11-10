//hudas code
#include "Pitchblade/panels/FormantPanel.h"
#include "Pitchblade/ui/ColorPalette.h"

FormantPanel::FormantPanel(AudioPluginAudioProcessor& proc)
    : processor(proc)
{
    // Formant toggle button - huda
    toggleViewButton.onClick = [this]()
        {
            showingFormants = !showingFormants;
            toggleViewButton.setButtonText(showingFormants ? "Hide Formants" : "Show Formants");
            repaint();
        };

    addAndMakeVisible(toggleViewButton);
    toggleViewButton.setButtonText(showingFormants ? "Hide Formants" : "Show Formants");

    startTimerHz(4); // repaint timer for the panel (visualizer has its own timer)

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

    // Create and add visualizer (hidden by default until toggled on)
    formantVisualizer = std::make_unique<FormantVisualizer>(processor, processor.apvts);
    addAndMakeVisible(formantVisualizer.get());
    formantVisualizer->setVisible(showingFormants);
}

void FormantPanel::resized()
{
    auto r = getLocalBounds().reduced(10);
    toggleViewButton.setBounds(r.removeFromTop(30));


    auto row1 = r.removeFromTop(40);
    formantLabel .setBounds(row1.removeFromLeft(90));
    formantSlider.setBounds(row1);

    auto row2 = r.removeFromTop(40);
    mixLabel.setBounds(row2.removeFromLeft(90));
    mixSlider.setBounds(row2);

    detectorArea = r; // visualizer area
    if (formantVisualizer)
        formantVisualizer->setBounds(detectorArea);

}

void FormantPanel::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);
}

//Button to show formants vs gain - huda
void FormantPanel::buttonClicked(juce::Button* button)
{
    if (button == &toggleViewButton)
    {
        showingFormants = !showingFormants;
        toggleViewButton.setButtonText(showingFormants ? "Hide Formants" : "Show Formants");
        if (formantVisualizer)
            formantVisualizer->setVisible(showingFormants);
        repaint();
    }
}

void FormantPanel::timerCallback() {
    if (showingFormants) { // refresh if formants are visible
        repaint();
    }
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
