#pragma once

#include <JuceHeader.h>
#include <vector>
#include <complex>

class FormantShifter
{
public:
    FormantShifter();
    ~FormantShifter() noexcept = default;

    // Call in prepareToPlay
    void prepare (double sampleRate, int maxBlockSize, int numChannels);

    // Call on transport reset / seek, or when SR/channels change
    void reset();

    // amount in [-50 .. +50], 0 = neutral
    // negative -> darker/deeper, positive -> brighter/chipmunky
    void setShiftAmount (float amount);

    // dry/wet in [0..1]
    // 0 = dry/original
    // 1 = fully shifted
    void setMix (float mix);

    // Main audio processing. In-place.
    void processBlock (juce::AudioBuffer<float>& buffer) noexcept;

private:
    // ----- constants -----
    static constexpr int   fftOrder   = 10;
    static constexpr int   fftSize    = 1 << fftOrder; // 1024
    static constexpr int   hopSize    = fftSize / 2;   // 512 (50% overlap)
    static constexpr float envSmooth  = 0.7f;          // envelope smoothing factor
    static constexpr int   maxCh      = 2;

    struct Channel
    {
        // input ring buffer
        juce::AudioBuffer<float> fifo; // mono [1 x fifoSize]
        int fifoWrite = 0;
        int fifoCount = 0;

        // overlap-add buffers
        juce::AudioBuffer<float> ola;       // mono [1 x olaSize]
        juce::AudioBuffer<float> olaWeight; // mono [1 x olaSize], accumulates window^2
        int olaWrite = 0;
        int olaRead  = 0;

        // per-frame analysis/synthesis work buffers
        juce::AudioBuffer<float> frameTime; // mono frame for FFT (fftSize)
        juce::HeapBlock<float>   window;    // Hann window [fftSize]
        juce::HeapBlock<juce::dsp::Complex<float>> fftIO;   // fftSize Complex bins
        juce::HeapBlock<float>   ifftTime;  // time-domain iFFT result [fftSize]

        juce::dsp::FFT fft { fftOrder };

        std::vector<float> mag;      // |X[k]|
        std::vector<float> phase;    // arg(X[k])
        std::vector<float> envOrig;  // smoothed log-mag envelope
        std::vector<float> envWarp;  // warped envelope (log)
        std::vector<std::complex<float>> specOut; // resynth spectrum

        void init (int fifoSize, int olaSize);

        static float antiDenorm (float v);

        // push new samples into FIFO
        void pushSamples (const float* in, int n);

        // do we have enough to process a hop?
        bool haveFrame() const;

        // grab fftSize samples ending hopSize ago, apply window
        void buildFrame();

        // run forward FFT -> fill mag[] and phase[]
        void forwardFFT();

        // crude envelope (lowpass in log-mag across frequency bins)
        void computeEnvelope();

        // warp that envelope in frequency by ratio
        void warpEnvelope (float ratio);

        // build shifted spectrum using warped envelope
        void buildShiftedSpectrum();

        // inverse FFT + apply window + overlap-add into OLA buffer + COLA weights
        void inverseFFTandOLA();

        // pull N samples from OLA into dst, divide by weights, consuming them
        void pull (float* dst, int n);
    };

    double sr   = 48000.0;
    int    nCh  = 1;
    float  shiftAmount = 0.0f; // [-50..50]
    float  mixVal      = 1.0f; // [0..1]

    Channel ch[maxCh];

    static float amountToRatio (float a);
};