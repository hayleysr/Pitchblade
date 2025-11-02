/**
 * Author: Hayley Spellicy-Ryan
 */
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/ui/ColorPalette.h"

// A visual part of a LevelMeter
class Level : public Component{
public:
    // Create level and set color
    Level(const Colour& color): color(color) {}
    void paint(juce::Graphics& g);
    // Set color status of each level part
    void setActive(bool active) { this->active = active; }
private:
    bool active;
    Colour color{};
};

// A collection of levels
class LevelMeter : public juce::Component, public juce::Timer{
public:
    // Initialize level meter with a link to a function that returns a float
    LevelMeter(std::function<float>&& valueFunction) : valueSupplier(std::move(valueFunction)) {}
    ~LevelMeter();

    void paint(juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;
private:
	std::function<float()> valueSupplier;	
    std::vector<std::unique_ptr<Level>> levels;
    const unsigned int numLevels = 10;
};