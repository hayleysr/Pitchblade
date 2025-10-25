#pragma once
#include <JuceHeader.h>
#include <vector>
#include <memory>

class FormantShifter
{
public:
    FormantShifter(); // impl constructed here
    ~FormantShifter() noexcept;  

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    // Controls
    void setShiftAmount(float amountNeg50toPos50) noexcept; // -50..+50 maps internally to 0.25..4.0x
    void setMix(float mix01) noexcept;                      // 0..1

    // Detector feed (per channel, Hz). For mono use ch=0 (and 1 if you want stereo copy).
    void setFormantFrequencies(int channel, const std::vector<float>& freqsHz);

    // Audio
    void processBlock(juce::AudioBuffer<float>& buffer) noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};