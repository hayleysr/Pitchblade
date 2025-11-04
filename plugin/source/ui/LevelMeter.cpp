#include "Pitchblade/ui/LevelMeter.h"

LevelMeter::LevelMeter(std::function<float()>&& valueFunction, float minRange, float maxRange, RotationMode rotationMode) : 
        valueSupplier(std::move(valueFunction)),
        sourceMin(minRange),
        sourceMax(maxRange),
        rotationMode(rotationMode)
{
    //todo: add in Austin's functionality with several Hz rates
    startTimerHz(30);
}

LevelMeter::~LevelMeter() {}

void LevelMeter::paint(juce::Graphics& g)
{
    const auto value = jmap(valueSupplier(), sourceMin, sourceMax, 0.f, 1.f);
    //Set each Level to active or not active
    for(unsigned int i = 0; i < numLevels; ++i)
    {
        if(value >= float(i + 1) / numLevels){
            levels[i]->setActive(true);
        }else{
            levels[i]->setActive(false);
        }
    }
}
void LevelMeter::resized()
{
    const auto bounds = getLocalBounds().toFloat();
    ColourGradient gradient{ Colors::accent, bounds.getBottomLeft(), Colors::buttonActive, bounds.getTopLeft(), false};
    gradient.addColour(0.5, Colors::panel);

    auto levelBounds = getLocalBounds();
    unsigned int levelWidth, levelHeight;
    if(rotationMode == RotationMode::RIGHT || RotationMode::LEFT)
        levelWidth = levelBounds.getHeight() / numLevels;
    else
        levelHeight = levelBounds.getWidth() / numLevels;

    levels.clear();
    for (auto i = 0; i < numLevels; i++)
    {
        auto level = std::make_unique<Level>(gradient.getColourAtPosition(static_cast<double>(i) / numLevels));
        addAndMakeVisible(level.get());
        if(rotationMode == RotationMode::RIGHT)
            level->setBounds(levelBounds.removeFromLeft(levelWidth));
        else if(rotationMode == RotationMode::LEFT)
            level->setBounds(levelBounds.removeFromRight(levelWidth));
        else if(rotationMode == RotationMode::DOWN)
            level->setBounds(levelBounds.removeFromTop(levelHeight));
        else
            level->setBounds(levelBounds.removeFromBottom(levelHeight));
        levels.push_back(std::move(level));
    }		
}

void LevelMeter::timerCallback()
{
    repaint();
}


void Level::paint(juce::Graphics& g)
{
    if(active){
        g.setColour(color);
    }else{
        g.setColour(Colors::background);
    }

    const float delta = 4.f;
    const auto bounds = getLocalBounds().toFloat().reduced(delta);
    const auto side = jmin(bounds.getWidth(), bounds.getHeight());
    const auto levelFillBounds = Rectangle<float>{ bounds.getX(), bounds.getY(), side, side };
    g.fillRoundedRectangle(levelFillBounds, delta);
    g.setColour(Colors::buttonActive);
    g.drawRoundedRectangle(levelFillBounds, delta, 1.f);
    if (active){
        g.setGradientFill(ColourGradient{ color.withAlpha(0.3f), levelFillBounds.getCentre(), color.withLightness(1.5f).withAlpha(0.f), {}, true });
        g.fillRoundedRectangle(levelFillBounds.expanded(delta), delta);
    }
}