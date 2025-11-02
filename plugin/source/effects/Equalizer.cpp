#include "Pitchblade/effects/Equalizer.h"
//Author: huda
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

    // scratch buffers per band
    lowBuf.setSize(channels, maxBlockSize);
    midBuf .setSize(channels, maxBlockSize);
    highBuf.setSize(channels, maxBlockSize);

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
}

void Equalizer::setMidFreq(float hz)
{
    midFreqHz = juce::jlimit(200.0f, 6000.0f, hz);
    updateFilters();
}

void Equalizer::setMidGainDb(float dB)
{
    midGainDb = juce::jlimit(-24.0f, 24.0f, dB);
}

void Equalizer::setHighFreq(float hz)
{
    highFreqHz = juce::jlimit(1000.0f, 18000.0f, hz);
    updateFilters();
}

void Equalizer::setHighGainDb(float dB)
{
    highGainDb = juce::jlimit(-24.0f, 24.0f, dB);
}

void Equalizer::updateFilters()
{
    if (!isPrepared)
        return;

    // basic 3-way split: lowpass, bandpass, highpass
    auto lowC = juce::dsp::IIR::Coefficients<float>::makeLowPass (sr, lowFreqHz .load());
    auto midC = juce::dsp::IIR::Coefficients<float>::makeBandPass(sr, midFreqHz .load(), midQ);
    auto highC= juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, highFreqHz.load());

    // push coeffs into each channel’s filter
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

    // copy input to each band’s buffer
    for (int ch = 0; ch < nCh; ++ch)
    {
        const float* in = buffer.getReadPointer(ch);
        lowBuf .copyFrom(ch, 0, in, nSmps);
        midBuf .copyFrom(ch, 0, in, nSmps);
        highBuf.copyFrom(ch, 0, in, nSmps);
    }

    // run IIRs per channel using AudioBlock slices
    juce::dsp::AudioBlock<float> bl(lowBuf);
    juce::dsp::AudioBlock<float> bm(midBuf);
    juce::dsp::AudioBlock<float> bh(highBuf);

    for (int ch = 0; ch < nCh; ++ch)
    {
        auto blCh = bl.getSingleChannelBlock((size_t)ch);
        auto bmCh = bm.getSingleChannelBlock((size_t)ch);
        auto bhCh = bh.getSingleChannelBlock((size_t)ch);

        lowBand .filters[(size_t)ch].process(juce::dsp::ProcessContextReplacing<float>(blCh));
        midBand .filters[(size_t)ch].process(juce::dsp::ProcessContextReplacing<float>(bmCh));
        highBand.filters[(size_t)ch].process(juce::dsp::ProcessContextReplacing<float>(bhCh));
    }

    // simple gains then add up the bands
    const float gL = dbToGain(lowGainDb .load());
    const float gM = dbToGain(midGainDb .load());
    const float gH = dbToGain(highGainDb.load());

    for (int ch = 0; ch < nCh; ++ch)
    {
        float* out = buffer.getWritePointer(ch);
        const float* l = lowBuf .getReadPointer(ch);
        const float* m = midBuf .getReadPointer(ch);
        const float* h = highBuf.getReadPointer(ch);

        for (int i = 0; i < nSmps; ++i)
            out[i] = gL * l[i] + gM * m[i] + gH * h[i];
    }

    // clear any channels we didn’t touch
    for (int ch = nCh; ch < buffer.getNumChannels(); ++ch)
        buffer.clear(ch, 0, nSmps);
}