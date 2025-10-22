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

    startTimerHz(4); // repaint timer 4 times per second - huda

    //startTimerHz(10); // repaint timer 10 times per second - huda

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
    toggleViewButton.setBounds(r.removeFromTop(30));


    auto row1 = r.removeFromTop(40);
    formantLabel .setBounds(row1.removeFromLeft(90));
    formantSlider.setBounds(row1);

    auto row2 = r.removeFromTop(40);
    mixLabel.setBounds(row2.removeFromLeft(90));
    mixSlider.setBounds(row2);

    detectorArea = r; // if you use an overlay for the detector lines

}

void FormantPanel::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);
    if (!showingFormants) return;

    // local copies: never mutate detectorArea
    auto header = detectorArea.withHeight(16);
    auto plot   = detectorArea.withTrimmedTop(16).reduced(2);

    // Title (smaller)
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(14.0f);
    g.drawText("Formant Detector Output", header, juce::Justification::centredLeft, false);

    // Frame
    g.setColour(juce::Colours::darkgrey.withAlpha(0.6f));
    g.drawRect(plot);

    // Formants
    const auto formantsCopy = processor.getLatestFormants();
    constexpr float fMin = 0.0f, fMax = 5000.0f;

    g.setColour(juce::Colours::red.withAlpha(0.95f));
    g.setFont(10.0f);

    for (float freqHz : formantsCopy)
    {
        const float f = juce::jlimit(fMin, fMax, freqHz);
        const float x = juce::jmap(f, fMin, fMax,
                                   (float)plot.getX(), (float)plot.getRight());

        g.drawLine(x, (float)plot.getY(), x, (float)plot.getBottom(), 2.0f);

        juce::Rectangle<int> tag((int)x - 26, plot.getBottom() - 14, 52, 12);
        g.setColour(juce::Colours::white);
        g.drawFittedText(juce::String(freqHz, 0) + " Hz", tag, juce::Justification::centred, 1);
        g.setColour(juce::Colours::red.withAlpha(0.95f));
    }
}

//Button to show formants vs gain - huda
void FormantPanel::buttonClicked(juce::Button* button)
{
    if (button == &toggleViewButton)
    {
        showingFormants = !showingFormants;
        toggleViewButton.setButtonText(showingFormants ? "Show Gain" : "Show Formants");
        //resized();
        repaint();
    }
}

void FormantPanel::timerCallback() {
    if (showingFormants) { // refresh if formants are visible
        repaint();
    }
}