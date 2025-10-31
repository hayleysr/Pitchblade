#include "Pitchblade/panels/PitchPanel.h"
#include "Pitchblade/ui/ColorPalette.h"

PitchPanel::PitchPanel(AudioPluginAudioProcessor& proc)
    : processor(proc)
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
    auto fifthBounds = bounds.reduced(bounds.getWidth() * 0.45, bounds.getHeight() * 0.45);
    auto radius = fifthBounds.getWidth() * 0.5f;
    g.fillEllipse(juce::Rectangle<float>(bounds.getCentre().x - radius, bounds.getCentre().y * 1.5f - radius, 
                    fifthBounds.getWidth(), fifthBounds.getWidth())); //x, y, w, h
    
}

void PitchPanel::drawDynamicLabels(juce::Graphics& g)
{
    auto fifthBounds = bounds.reduced(bounds.getWidth() * 0.45, bounds.getHeight() * 0.45);
    auto radius = fifthBounds.getWidth() * 0.5f;

    g.setFont(30.0f); 
    g.setColour(Colors::accent);
    g.drawText(processor.getPitchCorrector().getCurrentNoteName(), 
        0, juce::Justification::centred, 
        0, 0, juce::Justification::centred);
    g.drawText(processor.getPitchCorrector().getTargetNoteName(), 
        bounds.getCentre().x - radius, bounds.getCentre().y * 1.5f - radius,
        fifthBounds.getWidth(), fifthBounds.getWidth(), juce::Justification::centred);
    

    /*
    const juce::String &text, 
    int x, int y, 
    int width, int height, 
    juce::Justification justificationType, 
    bool useEllipsesIfTooBig = true
    */
}

void PitchPanel::timerCallback() {
    repaint();
}