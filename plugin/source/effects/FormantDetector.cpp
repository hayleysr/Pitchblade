#include "Pitchblade/effects/FormantDetector.h"
#include <algorithm>
#include <cmath>
#include <juce_dsp/juce_dsp.h>

//Author Huda

FormantDetector::FormantDetector(int order)
    : fftOrder(order),
      fftSize(1 << fftOrder), // fftSize = 2^fftOrder
      fft(fftOrder),
      window(fftSize, 0.0f),
      fftData(2 * fftSize, 0.0f)
{
    // Initialize Hann window: smooths edges to reduce spectral leakage
    for (int i = 0; i < fftSize; ++i)
        window[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (fftSize - 1)));
}

void FormantDetector::prepare(double sampleRateIn)
{
    // Prepare internal buffers and set sample rate
    sampleRate = sampleRateIn;
    formants.clear();
}

void FormantDetector::processBlock(const juce::AudioBuffer<float>& buffer)
{

    // Threshold for formants to stop detecting below a certain threshold
    float rms = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    if (rms < 1e-5f) {  // threshold for now and adjust if needed
        formants.clear();
        return;
    }


    // Compute FFT on incoming audio block
    computeFFT(buffer);

    // Find spectral peaks representing formants
    findFormantPeaks();
}

void FormantDetector::computeFFT(const juce::AudioBuffer<float>& buffer)
{
    // Copy first channel into fftData with windowing applied
    int numSamples = std::min(buffer.getNumSamples(), fftSize);
    auto* channelData = buffer.getReadPointer(0);

    for (int i = 0; i < numSamples; ++i)
        fftData[i] = channelData[i] * window[i];

    // Zero-pad if buffer is smaller than FFT size
    for (int i = numSamples; i < fftSize; ++i)
        fftData[i] = 0.0f;

    // Perform FFT (in-place, complex numbers interleaved as real/imag)
    fft.performRealOnlyForwardTransform(fftData.data());
}

void FormantDetector::findFormantPeaks()
{
    formants.clear();

    // Compute magnitude spectrum
    std::vector<float> magnitude(fftSize / 2);
    for (int i = 0; i < fftSize / 2; ++i)
    {
        float real = fftData[2 * i];
        float imag = fftData[2 * i + 1];
        magnitude[i] = std::sqrt(real * real + imag * imag);
    }

    // Find local maxima in the magnitude spectrum
    for (int i = 1; i < fftSize / 2 - 1; ++i)
    {
        if (magnitude[i] > magnitude[i - 1] && magnitude[i] > magnitude[i + 1])
            formants.push_back(static_cast<float>(i));
    }
}

std::vector<float> FormantDetector::getFormants() const
{
    return formants;
}

std::vector<float> FormantDetector::getFormantFrequencies() const
{
    std::vector<float> freqs;
    // Convert FFT bin indices to frequency in Hz
    for (auto bin : formants)
        freqs.push_back(bin * static_cast<float>(sampleRate) / static_cast<float>(fftSize));

    return freqs;
}

