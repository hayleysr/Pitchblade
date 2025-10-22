// Written by Austin Hills

#pragma once
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

class DeEsserProcessor{
private:
    //The level in dB that de-essing will start
    float threshold = 0.0f;
    //The amount of gain reduction
    float ratio = 4.0f;
    //Attack time in ms
    float attackTime = 5.0f;
    //Release time in ms
    float releaseTime = 5.0f;
    //The center frequency of the sibilance detector. Main de-esser control
    float frequency = 6000.0f;

    //Internal variables. See compressor.h for more information
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float envelope = 0.0f;
    double sampleRate = 44100.0;

    //A stereo IIR filter to detect sibilant frequencies in the sidechain
    //Use an array to hold one filter per channel
    std::array<juce::dsp::IIR::Filter<float>, 2> sidechainFilters;

    //Updates attack and release coefficients
    void updateAttackAndRelease();
    //Updates the sidechain filter coefficients when frequency or sample rate changes
    void updateFilter();
public:
    //Constructor
    DeEsserProcessor();

    //Called before processing to prepare the de-esser with the current sample rate
    void prepare(double sRate, int samplesPerBlock);

    //Setters
    void setThreshold(float thresholdDB);
    void setRatio(float ratioValue);
    void setAttack(float attackMs);
    void setRelease(float releaseMs);
    void setFrequency(float frequencyInHz);

    //Processes the input audio buffer to apply de-essing
    void process(juce::AudioBuffer<float>& buffer);
};