#include "Pitchblade/panels/PitchPanel.h"
#include "Pitchblade/ui/ColorPalette.h"

PitchPanel::PitchPanel(AudioPluginAudioProcessor& proc)
    : processor(proc),
    leftLevelMeter(std::make_unique<LevelMeter>([&]() {
    return std::abs(processor.getPitchCorrector().getCurrentPitch() -
                    processor.getPitchCorrector().getTargetPitch());})),
    rightLevelMeter(std::make_unique<LevelMeter>([&]() {
    return std::abs(processor.getPitchCorrector().getCurrentPitch() -
                    processor.getPitchCorrector().getTargetPitch());})),
{
    startTimerHz(8);    // Update 4x/second

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
    auto area = getLocalBounds().reduced(10);
}

void PitchPanel::paint(juce::Graphics& g)
{
    
    bounds = getLocalBounds();
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