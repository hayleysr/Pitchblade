#include "Pitchblade/panels/PitchPanel.h"
#include "Pitchblade/ui/ColorPalette.h"

PitchPanel::PitchPanel(AudioPluginAudioProcessor& proc)
    : processor(proc),
    leftLevelMeter(
        std::make_unique<LevelMeter>(
            [&](){
                return std::min(0.f, processor.getPitchCorrector().getSemitoneError());
            },
            0.f, -100.f, RotationMode::LEFT
        )
    ),
    rightLevelMeter(
        std::make_unique<LevelMeter>(
            [&](){
                return std::max(0.f, processor.getPitchCorrector().getSemitoneError());
            },
            0.f, 100.f, RotationMode::RIGHT
        )
    )
{
    startTimerHz(8);

    addAndMakeVisible(leftLevelMeter.get());
    addAndMakeVisible(rightLevelMeter.get());

    /*
    //doesn't update
        container that holds the pitch (3 boxes or circles)
        sliders
    //save for value tree:
        retune speed / smoothing
        retune ratio 
    //dropdown menu
        scale types (major, minor)
        scale values (from C to B)
    */
}

void PitchPanel::resized()
{
    auto localBounds = getLocalBounds().reduced(10);

    auto y = localBounds.getCentreY() + localBounds.getHeight() * 0.25;
    // juce::Rectangle<int> leftBounds (0, y + 100, localBounds.getWidth(), localBounds.getHeight());
    // juce::Rectangle<int> rightBounds (0, y, localBounds.getWidth(), localBounds.getHeight());

    // leftLevelMeter->setBounds(leftBounds);
    // rightLevelMeter->setBounds(rightBounds);

    auto leftArea  = localBounds.removeFromLeft(localBounds.getWidth() / 2).reduced(40);
    auto rightArea = localBounds.reduced(40);

    leftLevelMeter->setBounds(leftArea.withY(y).withHeight(localBounds.getHeight()));
    rightLevelMeter->setBounds(rightArea.withY(y).withHeight(localBounds.getHeight()));

}

void PitchPanel::paint(juce::Graphics& g)
{
    bounds = getLocalBounds().toFloat();
    drawStaticContent(g);
    drawDynamicLabels(g);
    /*
    //updates
        value of the target (biggest!), detected note name, and detected pitch in Hz 
        semitone bar 
    */
}

void PitchPanel::drawStaticContent(juce::Graphics& g)
{
    g.setColour(Colors::button);
    auto targetPitchDisplayBounds = bounds.reduced(bounds.getWidth() * 0.45, bounds.getHeight() * 0.45);
    auto radius = targetPitchDisplayBounds.getWidth() * 0.5f;
    g.fillEllipse(juce::Rectangle<float>(bounds.getCentre().x - radius, bounds.getCentre().y * 1.5f - radius, 
                    targetPitchDisplayBounds.getWidth(), targetPitchDisplayBounds.getWidth())); //x, y, w, h

    auto detectedPitchDisplayBounds = bounds.reduced(bounds.getWidth() * 0.45, bounds.getHeight() * 0.48);
    g.fillRect(juce::Rectangle<float>(bounds.getCentre().x - radius, bounds.getCentre().y * 1.25f + bounds.getCentre().y/2 - radius, 
                    targetPitchDisplayBounds.getWidth(), targetPitchDisplayBounds.getWidth())); //x, y, w, h)
    
}

void PitchPanel::drawDynamicLabels(juce::Graphics& g)
{
    auto targetPitchDisplayBounds = bounds.reduced(bounds.getWidth() * 0.45, bounds.getHeight() * 0.45);
    auto radius = targetPitchDisplayBounds.getWidth() * 0.5f;

    g.setFont(30.0f); 
    g.setColour(Colors::accent);
    g.drawText(processor.getPitchCorrector().getCurrentNoteName(), 
        0, juce::Justification::centred, 
        0, 0, juce::Justification::centred);
    g.drawText(processor.getPitchCorrector().getTargetNoteName(), 
        bounds.getCentre().x - radius, bounds.getCentre().y * 1.5f - radius,
        targetPitchDisplayBounds.getWidth(), targetPitchDisplayBounds.getWidth(), juce::Justification::centred);

    g.setFont(15.0f);
    auto detectedPitchDisplayBounds = bounds.reduced(bounds.getWidth() * 0.45, bounds.getHeight() * 0.48);
    g.drawText(processor.getPitchCorrector().getCurrentNoteName() + std::to_string(processor.getPitchCorrector().getCurrentPitch()),
        bounds.getCentre().x - radius, bounds.getCentre().y + bounds.getCentre().y/2 * 1.25f - radius, 
        targetPitchDisplayBounds.getWidth(), targetPitchDisplayBounds.getWidth(), juce::Justification::centred); //x, y, w, h)
    
}

void PitchPanel::timerCallback() {
    repaint();
}