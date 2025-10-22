//reyna 
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "Pitchblade/ui/ColorPalette.h"

//
struct CustomLookAndFeel : public juce::LookAndFeel_V4
{
    CustomLookAndFeel()
    {
        // base colours
        setColour(juce::ResizableWindow::backgroundColourId, Colors::background);
        setColour(juce::TextButton::buttonColourId, Colors::button);
        setColour(juce::TextButton::buttonOnColourId, Colors::button);
        setColour(juce::TextButton::textColourOffId, Colors::buttonText);
        setColour(juce::TextButton::textColourOnId, Colors::buttonActive);

        // sliders
        setColour(juce::Slider::backgroundColourId, Colors::button);      // track background
        setColour(juce::Slider::trackColourId, Colors::accent);           // filled track pink
        setColour(juce::Slider::thumbColourId, Colors::accent);           // knob pink
        setColour(juce::Slider::textBoxTextColourId, Colors::buttonText); // text
        setColour(juce::Slider::textBoxBackgroundColourId, Colors::panel);// textbox bg
        setColour(juce::Slider::textBoxOutlineColourId, Colors::accent);  // textbox outline
        
    }

    void drawPanelBackground(juce::Graphics& g, juce::Component& comp)
    {
        
        g.fillAll(Colors::panel);
        //g.setColour(Colors::accent);
        g.drawRect(comp.getLocalBounds(), 2);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        auto base = backgroundColour;

        if (shouldDrawButtonAsDown) base = base.darker();
        else if (shouldDrawButtonAsHighlighted) base = base.brighter();

        g.setColour(base);
        g.fillRoundedRectangle(bounds, 6.0f);

        // draw accent outline
        g.setColour(Colors::accent);
        g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
    }

    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override {
        // clear background
		g.fillAll(juce::Colours::transparentBlack); 
		// background
        g.setColour(Colors::panel.withAlpha(0.75f));
        g.fillRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, 6.0f);
		// outline
        g.setColour(Colors::panel);
        g.drawRoundedRectangle(0.5f, 0.5f, (float)width - 1.0f, (float)height - 1.0f, 6.0f, 1.5f);
        //text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f));
        g.drawFittedText(text, 8, 4, width - 16, height - 8, juce::Justification::centredLeft, 2);
    }
};