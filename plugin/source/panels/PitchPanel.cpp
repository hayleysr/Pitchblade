#include "Pitchblade/panels/PitchPanel.h"
#include "Pitchblade/ui/ColorPalette.h"

PitchPanel::PitchPanel(AudioPluginAudioProcessor& proc, juce::ValueTree& state)
    : processor(proc), localState(state),
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
    pitchName.setText("Pitch", juce::dontSendNotification);

    startTimerHz(8);

    // Render level meters
    addAndMakeVisible(leftLevelMeter.get());
    addAndMakeVisible(rightLevelMeter.get());

    // Sliders and their labels
    retuneSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    retuneSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    retuneSlider.setNumDecimalPlacesToDisplay(1);
    retuneSlider.setTextValueSuffix(" %");
    addAndMakeVisible(retuneSlider);

    retuneLabel.setText("Retune Speed", juce::dontSendNotification);
    retuneLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(retuneLabel);

    noteTransitionSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    noteTransitionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    noteTransitionSlider.setNumDecimalPlacesToDisplay(1);
    noteTransitionSlider.setTextValueSuffix(" cents");
    addAndMakeVisible(noteTransitionSlider);

    noteTransitionLabel.setText("Note Transition", juce::dontSendNotification);
    noteTransitionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(noteTransitionLabel);

    smoothingSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    smoothingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    smoothingSlider.setNumDecimalPlacesToDisplay(1);
    smoothingSlider.setTextValueSuffix(" %");
    addAndMakeVisible(smoothingSlider);

    smoothingLabel.setText("Correction Ratio", juce::dontSendNotification);
    smoothingLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(smoothingLabel);

    waverSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    waverSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    waverSlider.setNumDecimalPlacesToDisplay(1);
    waverSlider.setTextValueSuffix(" cents");
    addAndMakeVisible(waverSlider);

    waverLabel.setText("Waver", juce::dontSendNotification);
    waverLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(waverLabel); 

    // Link sliders to state
    const float startRetunePercent = (float)localState.getProperty("PitchRetune", 0.3f);
    retuneSlider.setRange(0.f, 1.f, 0.05f);
    retuneSlider.setValue(startRetunePercent, juce::dontSendNotification);
    retuneSlider.onValueChange = [this]() {
        localState.setProperty("PitchRetune", (float)retuneSlider.getValue(), nullptr);
        };

    const float noteTransitionCents = (float)localState.getProperty("PitchNoteTransition", 50.f);
    noteTransitionSlider.setRange(0.f, 50.f, 1.f);
    noteTransitionSlider.setValue(noteTransitionCents, juce::dontSendNotification);
    noteTransitionSlider.onValueChange = [this]() {
        localState.setProperty("PitchNoteTransition", (float)noteTransitionSlider.getValue(), nullptr);
        };

    const float smoothingPercent = (float)localState.getProperty("PitchSmoothing", 1.f);
    smoothingSlider.setRange(0.f, 1.f, 0.05f);
    smoothingSlider.setValue(smoothingPercent, juce::dontSendNotification);
    smoothingSlider.onValueChange = [this]() {
        localState.setProperty("PitchSmoothing", (float)smoothingSlider.getValue(), nullptr);
        };

    const float waverCents = (float)localState.getProperty("PitchWaver", 0.f);
    waverSlider.setRange(0.f, 20.f, 1.f);
    waverSlider.setValue(smoothingPercent, juce::dontSendNotification);
    waverSlider.onValueChange = [this]() {
        localState.setProperty("PitchWaver", (float)waverSlider.getValue(), nullptr);
        };

    localState.addListener(this);
}

void PitchPanel::resized()
{
    auto area = getLocalBounds().reduced(10);

    pitchName.setBounds(area.removeFromTop(area.getHeight() * 0.1f));

    auto y = area.getCentreY() + area.getHeight();

    auto scaleArea = area.removeFromTop(area.getHeight() * 0.1f);
    auto dialsArea = area.removeFromTop(area.getHeight() * 0.7f);
    auto metersArea = area;
    
    auto meterWidth = metersArea.getWidth() * 0.4f;
    auto leftArea  = metersArea.removeFromLeft(meterWidth).reduced(10);
    auto rightArea = metersArea.removeFromRight(meterWidth).reduced(10);

    leftLevelMeter->setBounds(leftArea);
    rightLevelMeter->setBounds(rightArea);

    // copied austin's formatting
    auto dials = dialsArea.reduced(10);
    int dialWidth = dials.getWidth() / 4;

    auto retuneArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto noteTransitionArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto smoothingArea = dials.removeFromLeft(dialWidth).reduced(5);
    auto waverArea = dials.reduced(5);

    retuneLabel.setBounds(retuneArea.removeFromTop(20));
    retuneSlider.setBounds(retuneArea);

    noteTransitionLabel.setBounds(noteTransitionArea.removeFromTop(20));
    noteTransitionSlider.setBounds(noteTransitionArea);

    smoothingLabel.setBounds(smoothingArea.removeFromTop(20));
    smoothingSlider.setBounds(smoothingArea);

    waverLabel.setBounds(waverArea.removeFromTop(20));
    waverSlider.setBounds(waverArea);

}

void PitchPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    drawStaticContent(g, bounds);
    drawDynamicLabels(g, bounds);
}

void PitchPanel::drawStaticContent(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(Colors::button);
    auto targetPitchDisplayBounds = bounds.reduced(bounds.getWidth() * 0.45, bounds.getHeight() * 0.45);
    auto radius = targetPitchDisplayBounds.getWidth() * 0.5f;
    g.fillEllipse({ bounds.getCentre().x - radius, 
                    bounds.getCentre().y * 1.5f - radius, 
                    targetPitchDisplayBounds.getWidth(), 
                    targetPitchDisplayBounds.getWidth()});

    auto detectedPitchDisplayBounds = bounds.reduced(bounds.getWidth() * 0.45, bounds.getHeight() * 0.48);
    g.fillRect(juce::Rectangle<float>(bounds.getCentre().x - radius, bounds.getCentre().y * 1.25f + bounds.getCentre().y/2 - radius, 
                    targetPitchDisplayBounds.getWidth(), targetPitchDisplayBounds.getWidth())); //x, y, w, h)
    
}

void PitchPanel::drawDynamicLabels(juce::Graphics& g, juce::Rectangle<float> bounds)
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

void PitchPanel::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property){
    if (tree == localState)
    {
        if (property == juce::Identifier("PitchRetune"))
            retuneSlider.setValue((float)tree.getProperty("PitchRetune"), juce::dontSendNotification);
        else if (property == juce::Identifier("PitchNoteTransition"))
            noteTransitionSlider.setValue((float)tree.getProperty("PitchNoteTransition"), juce::dontSendNotification);
        else if (property == juce::Identifier("PitchSmoothing"))
            smoothingSlider.setValue((float)tree.getProperty("PitchSmoothing"), juce::dontSendNotification);
        else if (property == juce::Identifier("PitchWaver"))
            waverSlider.setValue((float)tree.getProperty("PitchWaver"), juce::dontSendNotification);
    }
}