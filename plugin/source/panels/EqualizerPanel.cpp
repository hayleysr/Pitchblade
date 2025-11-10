// Huda and reyna
#include "Pitchblade/panels/EqualizerPanel.h"
#include <JuceHeader.h>
#include "Pitchblade/ui/ColorPalette.h"
#include <BinaryData.h>

// ===================== EqualizerPanel =====================
EqualizerPanel::EqualizerPanel (AudioPluginAudioProcessor& proc, juce::ValueTree& state)
    : processor(proc), localState(state)
      /*lowFreqAttachment   (processor.apvts, "EQ_LOW_FREQ",  lowFreq),
      lowGainAttachment   (processor.apvts, "EQ_LOW_GAIN",  lowGain),
      midFreqAttachment   (processor.apvts, "EQ_MID_FREQ",  midFreq),
      midGainAttachment   (processor.apvts, "EQ_MID_GAIN",  midGain),
      highFreqAttachment  (processor.apvts, "EQ_HIGH_FREQ", highFreq),
      highGainAttachment  (processor.apvts, "EQ_HIGH_GAIN", highGain)*/
{
    // Ranges mirror Equalizer.cpp limits
    setupKnob (lowFreq,  lowFreqLabel,  "Low Freq (Hz)",  20.0,   1000.0,  1.0,  false);
    setupKnob (lowGain,  lowGainLabel,  "Low Gain (dB)",  -24.0,     24.0, 0.1,  true);
    setupKnob (midFreq,  midFreqLabel,  "Mid Freq (Hz)",  200.0,   6000.0, 1.0,  false);
    setupKnob (midGain,  midGainLabel,  "Mid Gain (dB)",  -24.0,     24.0, 0.1,  true);
    setupKnob (highFreq, highFreqLabel, "High Freq (Hz)", 1000.0, 18000.0, 1.0,  false);
    setupKnob (highGain, highGainLabel, "High Gain (dB)", -24.0,     24.0, 0.1,  true);

    auto getProp = [&](const juce::String& key, float def) { return (float)localState.getProperty(key, def); };

    lowFreq.setValue(getProp("LowFreq", 100.0f), juce::dontSendNotification);
    lowGain.setValue(getProp("LowGain", 0.0f), juce::dontSendNotification);
    midFreq.setValue(getProp("MidFreq", 1000.0f), juce::dontSendNotification);
    midGain.setValue(getProp("MidGain", 0.0f), juce::dontSendNotification);
    highFreq.setValue(getProp("HighFreq", 5000.0f), juce::dontSendNotification);
    highGain.setValue(getProp("HighGain", 0.0f), juce::dontSendNotification);

    addAndMakeVisible (lowFreq);   addAndMakeVisible (lowFreqLabel);
    addAndMakeVisible (lowGain);   addAndMakeVisible (lowGainLabel);
    addAndMakeVisible (midFreq);   addAndMakeVisible (midFreqLabel);
    addAndMakeVisible (midGain);   addAndMakeVisible (midGainLabel);
    addAndMakeVisible (highFreq);  addAndMakeVisible (highFreqLabel);
    addAndMakeVisible (highGain);  addAndMakeVisible (highGainLabel);

    auto updateTree = [this](juce::Slider& s, const juce::String& key) {
        s.onValueChange = [this, &s, key]() {
            // Update local state for persistence/serialization
            localState.setProperty(key, (float)s.getValue(), nullptr);

            // Also push the value directly to the audio DSP via thread-safe setters
            // avoid reading/writing the ValueTree from the audio thread.
            if (key == "LowFreq")   processor.getEqualizer().setLowFreq((float)s.getValue());
            else if (key == "LowGain")   processor.getEqualizer().setLowGainDb((float)s.getValue());
            else if (key == "MidFreq")   processor.getEqualizer().setMidFreq((float)s.getValue());
            else if (key == "MidGain")   processor.getEqualizer().setMidGainDb((float)s.getValue());
            else if (key == "HighFreq")  processor.getEqualizer().setHighFreq((float)s.getValue());
            else if (key == "HighGain")  processor.getEqualizer().setHighGainDb((float)s.getValue());
        };
    };
    updateTree(lowFreq, "LowFreq");
    updateTree(lowGain, "LowGain");
    updateTree(midFreq, "MidFreq");
    updateTree(midGain, "MidGain");
    updateTree(highFreq, "HighFreq");
    updateTree(highGain, "HighGain");

    // Push initial slider values into the DSP so the Equalizer reflects stored node state immediately.
    processor.getEqualizer().setLowFreq((float)lowFreq.getValue());
    processor.getEqualizer().setLowGainDb((float)lowGain.getValue());
    processor.getEqualizer().setMidFreq((float)midFreq.getValue());
    processor.getEqualizer().setMidGainDb((float)midGain.getValue());
    processor.getEqualizer().setHighFreq((float)highFreq.getValue());
    processor.getEqualizer().setHighGainDb((float)highGain.getValue());

    localState.addListener(this);
}

void EqualizerPanel::resized()
{
    auto r = getLocalBounds().reduced (8);
    auto top = r.removeFromTop (r.getHeight() / 2).reduced (4);
    auto bot = r.reduced (4);

    auto w = top.getWidth() / 3;

    auto place = [] (juce::Rectangle<int> area, juce::Slider& s, juce::Label& l)
    {
        auto knob = area.removeFromTop (area.getHeight() - 20);
        s.setBounds (knob.reduced (8));
        l.setBounds (area);
    };

    // Top row: Frequency knobs
    auto c1 = top.removeFromLeft (w);
    auto c2 = top.removeFromLeft (w);
    auto c3 = top;
    place (c1, lowFreq,  lowFreqLabel);
    place (c2, midFreq,  midFreqLabel);
    place (c3, highFreq, highFreqLabel);

    // Bottom row: Gain knobs
    auto d1 = bot.removeFromLeft (w);
    auto d2 = bot.removeFromLeft (w);
    auto d3 = bot;
    place (d1, lowGain,  lowGainLabel);
    place (d2, midGain,  midGainLabel);
    place (d3, highGain, highGainLabel);
}

void EqualizerPanel::paint(juce::Graphics& g) {
    g.fillAll(Colors::background);
    g.drawRect(getLocalBounds(), 2);
}

void EqualizerPanel::setupKnob (juce::Slider& s, juce::Label& l, const juce::String& text,
                                double min, double max, double step, bool isGain)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
    s.setRange (min, max, step);
    s.setPopupMenuEnabled (true);
    s.setNumDecimalPlacesToDisplay (isGain ? 1 : 0);

    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
}

// Reyna 
//deconstructor
EqualizerPanel::~EqualizerPanel() {
    if (localState.isValid())
        localState.removeListener(this);
}

void EqualizerPanel::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (tree != localState) return;
    if (property == juce::Identifier("LowFreq"))   { auto v = (float)tree.getProperty("LowFreq");  lowFreq.setValue(v, juce::dontSendNotification);  processor.getEqualizer().setLowFreq(v); }
    if (property == juce::Identifier("LowGain"))   { auto v = (float)tree.getProperty("LowGain");  lowGain.setValue(v, juce::dontSendNotification);  processor.getEqualizer().setLowGainDb(v); }
    if (property == juce::Identifier("MidFreq"))   { auto v = (float)tree.getProperty("MidFreq");  midFreq.setValue(v, juce::dontSendNotification);  processor.getEqualizer().setMidFreq(v); }
    if (property == juce::Identifier("MidGain"))   { auto v = (float)tree.getProperty("MidGain");  midGain.setValue(v, juce::dontSendNotification);  processor.getEqualizer().setMidGainDb(v); }
    if (property == juce::Identifier("HighFreq"))  { auto v = (float)tree.getProperty("HighFreq"); highFreq.setValue(v, juce::dontSendNotification); processor.getEqualizer().setHighFreq(v); }
    if (property == juce::Identifier("HighGain"))  { auto v = (float)tree.getProperty("HighGain"); highGain.setValue(v, juce::dontSendNotification); processor.getEqualizer().setHighGainDb(v); }

}
