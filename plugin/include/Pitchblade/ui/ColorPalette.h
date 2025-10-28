//reyna macabebe
#pragma once
#include <JuceHeader.h>

// Color palette
namespace Colors
{
    inline const auto background   = juce::Colour::fromString("f393751"); // background
    inline const auto panel        = juce::Colour::fromString("ff19182b"); // panel bg

    inline const auto accent       = juce::Colour::fromString("fff551c1"); // outlines & active text
    inline const auto accentLight  = juce::Colour::fromString("ff686495"); // outlines & active text

    inline const auto button       = juce::Colour::fromString("ff19182b"); // button bg
    inline const auto buttonText   = juce::Colours::white;                 // button text normal
    inline const auto buttonActive = juce::Colour::fromString("fff551c1"); // button text active
}