#include "Pitchblade/effects/Equalizer.h"
//Author: huda
// More robust version with unity gain bypass

// tiny helper: dB to linear gain (floor at -60 dB to avoid denorm-ish stuff)
static inline float dbToGain(float dB)
{
    return juce::Decibels::decibelsToGain(dB, -60.0f);
}

void Equalizer::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    sr = sampleRate;
    channels = juce::jmax(1, numChannels);
    isPrepared = true;

    // allocate filter states per band/channel
    auto makeBand = [this](Band& b)
    {
        b.filters.clear();
        b.filters.resize((size_t)channels);
        for (auto& f : b.filters)
           { f.reset();}
    };

    makeBand(lowBand);
    makeBand(midBand);
    makeBand(highBand);

    // prepare each per channel duplicator for single channel processing
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = (juce::uint32)maxBlockSize;
    spec.numChannels = 1; // each duplicator instance handles one channel

    for (auto& f : lowBand.filters)  f.prepare(spec);
    for (auto& f : midBand.filters)  f.prepare(spec);
    for (auto& f : highBand.filters) f.prepare(spec);

    updateFilters();
}

void Equalizer::reset()
{
    for (auto& f : lowBand .filters) f.reset();
    for (auto& f : midBand .filters) f.reset();
    for (auto& f : highBand.filters) f.reset();
}

void Equalizer::setLowFreq(float hz)
{
    lowFreqHz = juce::jlimit(20.0f, 1000.0f, hz);
    updateFilters();
}

void Equalizer::setLowGainDb(float dB)
{
    lowGainDb = juce::jlimit(-24.0f, 24.0f, dB);
    updateFilters();
}

void Equalizer::setMidFreq(float hz)
{
    midFreqHz = juce::jlimit(200.0f, 6000.0f, hz);
    updateFilters();
}

void Equalizer::setMidGainDb(float dB)
{
    midGainDb = juce::jlimit(-24.0f, 24.0f, dB);
    updateFilters();
}

void Equalizer::setHighFreq(float hz)
{
    highFreqHz = juce::jlimit(1000.0f, 18000.0f, hz);
    updateFilters();
}

void Equalizer::setHighGainDb(float dB)
{
    highGainDb = juce::jlimit(-24.0f, 24.0f, dB);
    updateFilters();
}

void Equalizer::updateFilters()
{
    if (!isPrepared)
        return;

    float lowGain = lowGainDb.load();
    float midGain = midGainDb.load();
    float highGain = highGainDb.load();

    // Use shelving and peaking filters
    // For shelving filters: gainFactor is the linear gain, NOT dB
    auto lowC = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sr, lowFreqHz.load(), midQ, juce::Decibels::decibelsToGain(lowGain));

    auto midC = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, midFreqHz.load(), midQ, juce::Decibels::decibelsToGain(midGain));

    auto highC = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sr, highFreqHz.load(), midQ, juce::Decibels::decibelsToGain(highGain));

    // push coeffs into each channel's filter
    for (auto& f : lowBand .filters) f.state = lowC;
    for (auto& f : midBand .filters) f.state = midC;
    for (auto& f : highBand.filters) f.state = highC;
}

void Equalizer::processBlock(juce::AudioBuffer<float>& buffer) noexcept
{
    if (!isPrepared || buffer.getNumChannels() == 0)
        return;

    const int nCh   = juce::jmin(channels, buffer.getNumChannels());
    const int nSmps = buffer.getNumSamples();

    // Check if all gains are essentially zero - if so, just pass through
    float lowG = std::abs(lowGainDb.load());
    float midG = std::abs(midGainDb.load());
    float highG = std::abs(highGainDb.load());
    
    if (lowG < 0.01f && midG < 0.01f && highG < 0.01f)
    {
        // All gains near zero, pass through unchanged
        return;
    }

    juce::dsp::AudioBlock<float> block(buffer);

    // Apply each band's filter to each channel in series
    for (int ch = 0; ch < nCh; ++ch)
    {
        auto channelBlock = block.getSingleChannelBlock((size_t)ch);
        
        // Only process if gain is not near zero
        if (lowG > 0.01f)
        {
            lowBand.filters[(size_t)ch].process(
                juce::dsp::ProcessContextReplacing<float>(channelBlock));
        }
        
        if (midG > 0.01f)
        {
            midBand.filters[(size_t)ch].process(
                juce::dsp::ProcessContextReplacing<float>(channelBlock));
        }
        
        if (highG > 0.01f)
        {
            highBand.filters[(size_t)ch].process(
                juce::dsp::ProcessContextReplacing<float>(channelBlock));
        }
    }

    // clear any channels we didn't touch
    for (int ch = nCh; ch < buffer.getNumChannels(); ++ch)
        buffer.clear(ch, 0, nSmps);
}