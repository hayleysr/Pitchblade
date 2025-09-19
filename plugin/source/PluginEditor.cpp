#include <juce_core/juce_core.h>  // Or just include juce/juce.h

#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    //juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

//    // gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
//     //gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
//     gainSlider.setRange(-48.0, 48.0);
//     gainSlider.setValue(0.0);
//     gainSlider.addListener(this);
//     addAndMakeVisible(gainSlider);

    // Formant toggle button - huda
    toggleViewButton.addListener(this);
    addAndMakeVisible(toggleViewButton);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    toggleViewButton.removeListener(this);
    //gainSlider.removeListener(this);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    if (showingFormants) //huda
        {
            g.setColour(juce::Colours::white);
            g.setFont(18.0f);
            g.drawText("Formant Detector Output", getLocalBounds().withHeight(30), juce::Justification::centredTop);

            // Get formants from processor
            std::vector<float> formants = processorRef.getFormantDetector().getFormants();

            // Draw red lines
            g.setColour(juce::Colours::red);
            auto width = getWidth();
            auto height = getHeight();
            for (float f : formants)
            {
                float x = juce::jmap(f, 0.0f, 5000.0f, 0.0f, static_cast<float>(width));
                g.drawLine(x, 40.0f, x, height - 20.0f, 2.0f);
            }

            // Hello world message (for now)
            g.setColour(juce::Colours::yellow);
            g.setFont(14.0f);
            g.drawText("Hello World, here are the dominant frequencies!",
                    getLocalBounds().withHeight(20).translated(0, height - 30),
                    juce::Justification::centredBottom);
        }
    
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    toggleViewButton.setBounds(10, 10, 120, 30);// huda

    if (!showingFormants) // So that gain GUI isn't affected much - huda
    {
        gainSlider.setBounds(getLocalBounds().reduced(50));
        gainSlider.setVisible(true);
    }
    else
    {
        gainSlider.setVisible(false);
    }
}

void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainSlider)
    {
        processorRef.gainDB = (float)gainSlider.getValue();
    }
}

//Button to show formants vs gain - huda
void AudioPluginAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &toggleViewButton)
    {
        showingFormants = !showingFormants;
        toggleViewButton.setButtonText(showingFormants ? "Show Gain" : "Show Formants");
        repaint();
    }
}