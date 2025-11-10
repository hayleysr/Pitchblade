// EQ response visualizer using FrequencyGraphVisualizer

#include "Pitchblade/ui/EqualizerVisualizer.h"
#include "Pitchblade/effects/Equalizer.h"

EqualizerVisualizer::EqualizerVisualizer(AudioPluginAudioProcessor& proc)
    : processor(proc)
{
    // Use displayMode 1 to draw a horizontal 0 dB line and a vertical band marker
    // FrequencyGraphVisualizer y-axis is fixed to [-100, 0] dB.
    graph = std::make_unique<FrequencyGraphVisualizer>(processor.apvts, 5, 1);
    addAndMakeVisible(*graph);

    // Initial threshold: show 0 dB line and mid band center as a vertical guide
    graph->setThreshold(processor.getEqualizer().getMidFreq(), 0.0f);

    // Update regularly; cost is low (rebuilding 300 points)
    // FrequencyGraphVisualizer handles its own repaint timer; we just refresh data
    startTimerHz(15);

    // Populate once immediately
    updateResponseCurve();
}

EqualizerVisualizer::~EqualizerVisualizer()
{
    stopTimer();
}

void EqualizerVisualizer::resized()
{
    if (graph)
        graph->setBounds(getLocalBounds());
}

void EqualizerVisualizer::timerCallback()
{
    // Keep the vertical threshold in sync with the mid frequency
    graph->setThreshold(processor.getEqualizer().getMidFreq(), 0.0f);
    updateResponseCurve();
}

void EqualizerVisualizer::buildLogFrequencies(std::vector<float>& freqs, int numPoints)
{
    freqs.clear();
    freqs.reserve((size_t)numPoints);
    const float fStart = 20.0f;
    const float fEnd   = 20000.0f;
    const float logStart = std::log10(fStart);
    const float logEnd   = std::log10(fEnd);
    for (int i = 0; i < numPoints; ++i)
    {
        float t = (numPoints <= 1 ? 0.0f : (float)i / (float)(numPoints - 1));
        float f = std::pow(10.0f, juce::jmap(t, logStart, logEnd));
        freqs.push_back(f);
    }
}

void EqualizerVisualizer::updateResponseCurve()
{
    auto& eq = processor.getEqualizer();

    // Get current parameters (thread-safe atomics)
    const float lowHz   = eq.getLowFreq();
    const float midHz   = eq.getMidFreq();
    const float highHz  = eq.getHighFreq();
    const float lowDb   = eq.getLowGainDb();
    const float midDb   = eq.getMidGainDb();
    const float highDb  = eq.getHighGainDb();

    // Sample rate may be 0 before prepareToPlay; default to 44100
    double sr = processor.getSampleRate();
    if (sr <= 0.0)
        sr = 44100.0;

    // Build coefficients mirroring the DSP path (Q=1.0 to match Equalizer)
    const float Q = 1.0f;
    auto lowC  = juce::dsp::IIR::Coefficients<float>::makeLowShelf (sr, lowHz,  Q, juce::Decibels::decibelsToGain(lowDb));
    auto midC  = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr, midHz, Q, juce::Decibels::decibelsToGain(midDb));
    auto highC = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr, highHz, Q, juce::Decibels::decibelsToGain(highDb));

    // Generate response over log-spaced frequencies
    std::vector<float> freqs;
    buildLogFrequencies(freqs, 300);

    std::vector<juce::Point<float>> points;
    points.reserve(freqs.size());

    for (float f : freqs)
    {
        // Combine magnitudes in linear domain
        float mag = 1.0f;
        if (lowC)  mag *= lowC ->getMagnitudeForFrequency(f, (float)sr);
        if (midC)  mag *= midC ->getMagnitudeForFrequency(f, (float)sr);
        if (highC) mag *= highC->getMagnitudeForFrequency(f, (float)sr);

        // Convert to dB; the FrequencyGraphVisualizer expects [-100, 0] dB range.
        float db = juce::Decibels::gainToDecibels(mag, -100.0f);
        db = juce::jlimit(-100.0f, 0.0f, db);

        points.emplace_back(f, db);
    }

    // Push to the graph (thread-safe inside the visualizer)
    graph->updateSpectrumData(points);
}
