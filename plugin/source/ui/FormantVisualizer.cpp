// huda
#include "Pitchblade/ui/FormantVisualizer.h"

//=========================== Overlay ===========================
void FormantVisualizer::FormantOverlay::paint(juce::Graphics& g)
{
    // Transparent background; draw only markers over the grid
    auto bounds = getLocalBounds();
    auto graph = bounds.reduced(0);
    // align with FrequencyGraphVisualizer layout
    auto xLabels = graph.removeFromBottom(labelHeight);
    auto yLabels = graph.removeFromLeft(labelWidth);

    // Now graph contains the plotting area
    const auto& freqs = proc.getLatestFormants();
    if (freqs.empty()) return;

    g.setColour(Colors::accent);
    g.setFont(10.0f);

    for (float fHz : freqs)
    {
        float f = juce::jlimit(xAxisHz.getStart(), xAxisHz.getEnd(), fHz);
        float x = mapFreqToX(f, graph);

        g.drawLine(x, (float)graph.getY(), x, (float)graph.getBottom(), 2.0f);

        auto tag = juce::Rectangle<int>((int)x - 28, graph.getY() + 4, 56, 14);
        g.setColour(juce::Colours::white);
        g.drawFittedText(juce::String(fHz, 0) + " Hz", tag, juce::Justification::centred, 1);
        g.setColour(Colors::accent);
    }
}

float FormantVisualizer::FormantOverlay::mapFreqToX(float freq, juce::Rectangle<int> graph) const
{
    auto clamped = juce::jlimit(xAxisHz.getStart(), xAxisHz.getEnd(), freq);
    float lf = std::log10(clamped);
    float proportion = (lf - logFreqStart) / (logFreqEnd - logFreqStart);
    return juce::jmap(proportion, (float)graph.getX(), (float)graph.getRight());
}

//=========================== Container ===========================
FormantVisualizer::FormantVisualizer(AudioPluginAudioProcessor& processorRef,
                                     juce::AudioProcessorValueTreeState& vts)
    : processor(processorRef), apvts(vts)
{
    // Background grid/axes from FrequencyGraphVisualizer
    freqGraph = std::make_unique<FrequencyGraphVisualizer>(apvts, 5, 0);
    addAndMakeVisible(freqGraph.get());

    // Overlay markers
    overlay = std::make_unique<FormantOverlay>(processor);
    addAndMakeVisible(overlay.get());

    // Listen to global framerate and start timer accordingly
    apvts.addParameterListener("GLOBAL_FRAMERATE", this);

    int initialIndex = apvts.getRawParameterValue("GLOBAL_FRAMERATE")
                         ? (int) *apvts.getRawParameterValue("GLOBAL_FRAMERATE")
                         : 2; // default ~30 Hz

    switch (initialIndex)
    {
        case 0: startTimerHz(5);  break;
        case 1: startTimerHz(15); break;
        case 2: startTimerHz(30); break;
        case 3: startTimerHz(60); break;
        default: startTimerHz(30); break;
    }
}

FormantVisualizer::~FormantVisualizer()
{
    stopTimer();
    apvts.removeParameterListener("GLOBAL_FRAMERATE", this);
}

void FormantVisualizer::resized()
{
    auto b = getLocalBounds();
    if (freqGraph) freqGraph->setBounds(b);
    if (overlay)   overlay->setBounds(b);
}

void FormantVisualizer::paint(juce::Graphics& g)
{
    // Container paints nothing; children handle drawing
}

void FormantVisualizer::timerCallback()
{
    if (isShowing() && overlay)
        overlay->repaint();
}

void FormantVisualizer::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID != "GLOBAL_FRAMERATE")
        return;

    int index = (int) newValue;
    switch (index)
    {
        case 0: startTimerHz(5);  break;
        case 1: startTimerHz(15); break;
        case 2: startTimerHz(30); break;
        case 3: startTimerHz(60); break;
        default: startTimerHz(30); break;
    }
}
