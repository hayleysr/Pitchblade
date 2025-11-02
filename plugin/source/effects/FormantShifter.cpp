#include "Pitchblade/effects/FormantShifter.h"
#include <cmath>
#include <algorithm>

// map UI [-50..+50] -> ratio [0.25..4.0]
float FormantShifter::amountToRatio (float a)
{
    float t = juce::jlimit(-1.0f, 1.0f, a / 50.0f);    // -50 -> -1, +50 -> +1
    float r = std::pow(2.0f, 2.0f * t);                // -1 -> 0.25x, +1 -> 4.0x
    // Clamp extremes while you test; widen later if desired:
    return juce::jlimit(0.5f, 2.0f, r);
}

//============================================================
// Channel helpers
//============================================================

void FormantShifter::Channel::init (int fifoSize, int olaSize)
{
    fifo.setSize(1, fifoSize, false, false, true);
    fifo.clear();
    fifoWrite = 0;
    fifoCount = 0;

    ola.setSize(1, olaSize, false, false, true);
    ola.clear();
    olaWeight.setSize(1, olaSize, false, false, true);
    olaWeight.clear();
    olaWrite = 0;
    olaRead  = 0;

    frameTime.setSize(1, fftSize, false, false, true);
    frameTime.clear();

    window.allocate(fftSize, true);
    fftIO.allocate(fftSize, true);   // Complex bins
    ifftTime.allocate(fftSize, true);

    mag    .assign(fftSize, 0.0f);
    phase  .assign(fftSize, 0.0f);
    envOrig.assign(fftSize, 0.0f);
    envWarp.assign(fftSize, 0.0f);
    specOut.assign(fftSize, std::complex<float>(0.0f, 0.0f));

    // Hann window
    for (int n = 0; n < fftSize; ++n)
        window[n] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * (float)n
                                            / (float)(fftSize - 1)));
}

float FormantShifter::Channel::antiDenorm (float v)
{
    return (std::abs(v) < 1.0e-30f ? 0.0f : v);
}

void FormantShifter::Channel::pushSamples (const float* in, int n)
{
    float* f = fifo.getWritePointer(0);
    const int fs = fifo.getNumSamples();

    for (int i = 0; i < n; ++i)
    {
        f[fifoWrite] = in[i];
        fifoWrite = (fifoWrite + 1) % fs;
        fifoCount = std::min(fifoCount + 1, fs);
    }
}

bool FormantShifter::Channel::haveFrame() const
{
    return fifoCount >= fftSize; //to have the full frame availbale
}

void FormantShifter::Channel::buildFrame()
{
    float* dst = frameTime.getWritePointer(0);
    const float* f = fifo.getReadPointer(0);
    const int fs = fifo.getNumSamples();

    // we take fftSize samples ending hopSize behind fifoWrite
    int endPos = fifoWrite - hopSize;
    while (endPos < 0) endPos += fs;

    for (int n = 0; n < fftSize; ++n)
    {
        int idx = endPos - (fftSize - 1 - n);
        while (idx < 0)   idx += fs;
        while (idx >= fs) idx -= fs;

        dst[n] = f[idx] * window[n];
    }

    // consume hopSize samples
    fifoCount = std::max(0, fifoCount - hopSize);
}

void FormantShifter::Channel::forwardFFT()
{
    auto* io  = fftIO.getData();
    auto* tim = frameTime.getReadPointer(0);

    // time -> complex buffer
    for (int n = 0; n < fftSize; ++n)
    {
        io[n].real(tim[n]);
        io[n].imag(0.0f);
    }

    // forward FFT in-place
    fft.perform(io, io, false);

    for (int k = 0; k < fftSize; ++k)
    {
        const float re = io[k].real();
        const float im = io[k].imag();
        const float m  = std::sqrt(re*re + im*im) + 1.0e-24f;

        mag[k]   = m;
        phase[k] = std::atan2(im, re);
    }
}

void FormantShifter::Channel::computeEnvelope()
{
    // simple 1-pole LP on log|X|
    float prev = std::log(mag[0]);
    envOrig[0] = prev;

    for (int k = 1; k < fftSize; ++k)
    {
        float here = std::log(mag[k]);
        prev = envSmooth * prev + (1.0f - envSmooth) * here;
        envOrig[k] = prev;
    }
}

void FormantShifter::Channel::warpEnvelope (float ratio)
{
    const float lastBin = (float)(fftSize - 1);

    for (int k = 0; k < fftSize; ++k)
    {
        float src = (float) k / ratio;
        if (src < 0.0f)      src = 0.0f;
        if (src > lastBin)   src = lastBin;

        int   i0 = (int) std::floor(src);
        int   i1 = juce::jmin(i0 + 1, fftSize - 1);
        float f  = src - (float) i0;

        float v0 = envOrig[i0];
        float v1 = envOrig[i1];
        envWarp[k] = v0 + f * (v1 - v0);
    }
}

void FormantShifter::Channel::buildShiftedSpectrum()
{
    for (int k = 0; k < fftSize; ++k)
    {
        float logMag    = std::log(mag[k] + 1.0e-24f);
        float logNewMag = logMag - envOrig[k] + envWarp[k];
        float newMag    = std::exp(logNewMag);

        float ph = phase[k];
        float re = newMag * std::cos(ph);
        float im = newMag * std::sin(ph);

        specOut[k].real(re);
        specOut[k].imag(im);
    }
}

void FormantShifter::Channel::inverseFFTandOLA()
{
    auto* io = fftIO.getData();

    // move specOut (std::complex<float>) into fftIO (juce::dsp::Complex<float>)
    for (int k = 0; k < fftSize; ++k)
    {
        io[k].real(specOut[k].real());
        io[k].imag(specOut[k].imag());
    }

    // inverse FFT in-place (JUCE does NOT normalise)
    fft.perform(io, io, true);

    // time-domain, apply window AND 1/fftSize scaling
    float* outTD = ifftTime.getData();
    const float invN = 1.0f / (float) fftSize;
    for (int n = 0; n < fftSize; ++n)
        outTD[n] = io[n].real() * window[n] * invN;

    float* olaData = ola.getWritePointer(0);
    float* wData   = olaWeight.getWritePointer(0);
    const int os   = ola.getNumSamples();

    // overlap-add AND accumulate window^2 weights
    for (int n = 0; n < fftSize; ++n)
    {
        const int idx = (olaWrite + n) % os;
        olaData[idx] += outTD[n];
        wData[idx]   += window[n] * window[n];
    }

    // advance write pointer by hop
    olaWrite = (olaWrite + hopSize) % os;
}

void FormantShifter::Channel::pull (float* dst, int n)
{
    float* olaData = ola.getWritePointer(0);
    float* wData   = olaWeight.getWritePointer(0);
    const int os   = ola.getNumSamples();

    for (int i = 0; i < n; ++i)
    {
        const float w = wData[olaRead];
        float v = (w > 1.0e-12f) ? (olaData[olaRead] / w) : 0.0f;

        dst[i] = std::isfinite(v) ? antiDenorm(v) : 0.0f;

        // consume
        olaData[olaRead] = 0.0f;
        wData[olaRead]   = 0.0f;

        olaRead = (olaRead + 1) % os;
    }
}

//============================================================
// FormantShifter public
//============================================================

FormantShifter::FormantShifter()
{
    // choose buffer sizes (independent of prepare() so they're allocated up front)
    const int fifoSize = fftSize * 2;
    const int olaSize  = fftSize * 4;

    for (int c = 0; c < maxCh; ++c)
        ch[c].init(fifoSize, olaSize);
}

void FormantShifter::prepare (double sampleRate, int /*maxBlockSize*/, int numChannels)
{
    sr  = sampleRate;
    nCh = juce::jlimit(1, maxCh, numChannels);
    reset();
}

void FormantShifter::reset()
{
    // re-init clears state
    const int fifoSize = fftSize * 2;
    const int olaSize  = fftSize * 4;

    for (int c = 0; c < nCh; ++c)
        ch[c].init(fifoSize, olaSize);
}

void FormantShifter::setShiftAmount (float amount)
{
    shiftAmount = juce::jlimit(-50.0f, 50.0f, amount);
}

void FormantShifter::setMix (float mix)
{
    mixVal = juce::jlimit(0.0f, 1.0f, mix);
}

void FormantShifter::processBlock (juce::AudioBuffer<float>& buffer) noexcept
{
    const int numSamples = buffer.getNumSamples();
    const int chans      = std::min(nCh, buffer.getNumChannels());
    if (numSamples <= 0 || chans <= 0)
        return;

    // keep dry for mix
    juce::AudioBuffer<float> dryCopy;
    dryCopy.makeCopyOf(buffer, true);

    // push incoming audio into per-channel FIFOs
    for (int c = 0; c < chans; ++c)
        ch[c].pushSamples(buffer.getReadPointer(c), numSamples);

    // generate as many new shifted frames as possible
    const float ratio = amountToRatio(shiftAmount);

    for (;;)
    {
        bool allReady = true;
        for (int c = 0; c < chans; ++c)
            allReady = allReady && ch[c].haveFrame();

        if (!allReady)
            break;

        for (int c = 0; c < chans; ++c)
        {
            auto& cc = ch[c];
            cc.buildFrame();
            cc.forwardFFT();
            cc.computeEnvelope();
            cc.warpEnvelope(ratio);
            cc.buildShiftedSpectrum();
            cc.inverseFFTandOLA();
        }
    }

    // pull wet audio and mix with dry
    for (int c = 0; c < chans; ++c)
    {
        juce::HeapBlock<float> wetTmp;
        wetTmp.allocate(numSamples, true);
        ch[c].pull(wetTmp.getData(), numSamples);

        float* out = buffer.getWritePointer(c);
        const float* dry = dryCopy.getReadPointer(c);

        for (int i = 0; i < numSamples; ++i)
        {
            const float y = (1.0f - mixVal) * dry[i] + mixVal * wetTmp[i];
            out[i] = std::isfinite(y) ? y : 0.0f;
        }
    }

    // clear any channels beyond nCh
    for (int c = chans; c < buffer.getNumChannels(); ++c)
        buffer.clear(c, 0, numSamples);
}