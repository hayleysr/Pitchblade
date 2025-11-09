#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <vector>

/* Author: huda
   Equalizer; basic 3-band EQ:
     - low  = low shelf (cutoff + gain)
     - mid  = peaking (center + gain)
     - high = high shelf (cutoff + gain)
   Uses proper shelving filters for transparent EQ.
*/

class Equalizer
{
public:
    Equalizer() = default;

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    // param setters (safe from GUI thread)
    void setLowFreq(float hz);
    void setLowGainDb(float dB);

    void setMidFreq(float hz);
    void setMidGainDb(float dB);

    void setHighFreq(float hz);
    void setHighGainDb(float dB);

    // process in-place
    void processBlock(juce::AudioBuffer<float>& buffer) noexcept;

    // getters for UI
    float getLowFreq() const noexcept { return lowFreqHz;  }
    float getLowGainDb()const noexcept { return lowGainDb;  }
    float getMidFreq()  const noexcept { return midFreqHz;  }
    float getMidGainDb()const noexcept { return midGainDb;  }
    float getHighFreq() const noexcept { return highFreqHz; }
    float getHighGainDb() const noexcept { return highGainDb; }

private:
    void updateFilters();

    double sr = 44100.0;
    int channels = 2;
    bool isPrepared = false;

    // knobs
    std::atomic<float> lowFreqHz  { 200.0f };
    std::atomic<float> lowGainDb  {0.0f };

    std::atomic<float> midFreqHz  { 1000.0f };
    std::atomic<float> midGainDb  { 0.0f };
    const float midQ = 1.0f; // fixed Q for now

    std::atomic<float> highFreqHz { 4000.0f };
    std::atomic<float> highGainDb {0.0f };

    // smoothing for gain changes (applied on audio thread)
    float smoothingTimeSeconds = 0.02f; // 20 ms
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> lowGainSmooth;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> midGainSmooth;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> highGainSmooth;

    // one ProcessorDuplicator per channel per band
    using IIRFilter = juce::dsp::IIR::Filter<float>;
    using IIRCoeff = juce::dsp::IIR::Coefficients<float>;
    using IIRProc= juce::dsp::ProcessorDuplicator<IIRFilter, IIRCoeff>;

    struct Band
    {
        std::vector<IIRProc> filters;
    };

    Band lowBand, midBand, highBand;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Equalizer)
};
