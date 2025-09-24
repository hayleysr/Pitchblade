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
}



void FormantPanel::resized()
{
    auto area = getLocalBounds().reduced(10);
    toggleViewButton.setBounds(area.removeFromTop(30));
    //gainSlider.setBounds(area.removeFromTop(40));
}

void FormantPanel::paint(juce::Graphics& g) {
    if (showingFormants) //huda
    {
        g.fillAll(Colors::background);
        g.setColour(juce::Colours::white);
        g.setFont(18.0f);
        g.drawText("Formant Detector Output", 0,50, getWidth(), 50, juce::Justification::centredTop);

        // Get formants from processor
        //std::vector<float> formants = processorRef.getFormantDetector().getFormants();
        auto formants = processor.getFormantDetector().getFormants();

        // Draw red lines
        g.setColour(juce::Colours::red);
        auto width = getWidth();
        auto height = getHeight();
        for (float freqHz : processor.getLatestFormants()) {
            float x = juce::jmap(freqHz, 0.0f, 1500.0f, 0.0f, (float)width);

            g.drawLine(x, 40.0f, x, height - 20.0f, 2.0f);

            g.setColour(juce::Colours::white);
            g.setFont(12.0f);
            g.drawText(juce::String::String(freqHz, 0, false) + "Hz",
                juce::Rectangle<int>(int(x) - 30, height - 35, 60, 20),
                juce::Justification::centred);
            g.setColour(juce::Colours::red);

        }

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