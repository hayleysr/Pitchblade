#include "Pitchblade/panels/EqualizerPanel.h"

// ===================== EqualizerPanel =====================
EqualizerPanel::EqualizerPanel (AudioPluginAudioProcessor& proc)
    : processor(proc),
      lowFreqAttachment   (processor.apvts, "EQ_LOW_FREQ",  lowFreq),
      lowGainAttachment   (processor.apvts, "EQ_LOW_GAIN",  lowGain),
      midFreqAttachment   (processor.apvts, "EQ_MID_FREQ",  midFreq),
      midGainAttachment   (processor.apvts, "EQ_MID_GAIN",  midGain),
      highFreqAttachment  (processor.apvts, "EQ_HIGH_FREQ", highFreq),
      highGainAttachment  (processor.apvts, "EQ_HIGH_GAIN", highGain)
{
    // Ranges mirror Equalizer.cpp limits
    setupKnob (lowFreq,  lowFreqLabel,  "Low Freq (Hz)",  20.0,   1000.0,  1.0,  false);
    setupKnob (lowGain,  lowGainLabel,  "Low Gain (dB)",  -24.0,     24.0, 0.1,  true);
    setupKnob (midFreq,  midFreqLabel,  "Mid Freq (Hz)",  200.0,   6000.0, 1.0,  false);
    setupKnob (midGain,  midGainLabel,  "Mid Gain (dB)",  -24.0,     24.0, 0.1,  true);
    setupKnob (highFreq, highFreqLabel, "High Freq (Hz)", 1000.0, 18000.0, 1.0,  false);
    setupKnob (highGain, highGainLabel, "High Gain (dB)", -24.0,     24.0, 0.1,  true);

    addAndMakeVisible (lowFreq);   addAndMakeVisible (lowFreqLabel);
    addAndMakeVisible (lowGain);   addAndMakeVisible (lowGainLabel);
    addAndMakeVisible (midFreq);   addAndMakeVisible (midFreqLabel);
    addAndMakeVisible (midGain);   addAndMakeVisible (midGainLabel);
    addAndMakeVisible (highFreq);  addAndMakeVisible (highFreqLabel);
    addAndMakeVisible (highGain);  addAndMakeVisible (highGainLabel);
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

// ===================== EqualizerNode =====================
EqualizerNode::EqualizerNode (AudioPluginAudioProcessor& proc)
    : EffectNode (proc, "EqualizerNode", "Equalizer")
{}

EqualizerNode::EqualizerNode (AudioPluginAudioProcessor& proc, const juce::ValueTree& state)
    : EffectNode (proc, state)
{}

std::unique_ptr<juce::Component> EqualizerNode::createPanel (AudioPluginAudioProcessor& proc)
{
    return std::make_unique<EqualizerPanel>(proc);
}

void EqualizerNode::process (AudioPluginAudioProcessor& proc, juce::AudioBuffer<float>& buffer)
{
    // Pull APVTS params → push into DSP
    if (auto* v = proc.apvts.getRawParameterValue("EQ_LOW_FREQ"))   proc.getEqualizer().setLowFreq   (v->load());
    if (auto* v = proc.apvts.getRawParameterValue("EQ_LOW_GAIN"))   proc.getEqualizer().setLowGainDb (v->load());
    if (auto* v = proc.apvts.getRawParameterValue("EQ_MID_FREQ"))   proc.getEqualizer().setMidFreq   (v->load());
    if (auto* v = proc.apvts.getRawParameterValue("EQ_MID_GAIN"))   proc.getEqualizer().setMidGainDb (v->load());
    if (auto* v = proc.apvts.getRawParameterValue("EQ_HIGH_FREQ"))  proc.getEqualizer().setHighFreq  (v->load());
    if (auto* v = proc.apvts.getRawParameterValue("EQ_HIGH_GAIN"))  proc.getEqualizer().setHighGainDb(v->load());

    proc.getEqualizer().processBlock(buffer);
}

std::shared_ptr<EffectNode> EqualizerNode::clone() const
{
    auto n = std::make_shared<EqualizerNode>(processor);
    n->bypassed = bypassed;
    n->setDisplayName(effectName);
    return n;
}