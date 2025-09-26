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
};