/**
 * Author: Hayley Spellicy-Ryan
 * PitchShifter class
 * -------------------
 * Rubberband processor
 */

#pragma once
#include <JuceHeader.h>
#include "../../third-party/rubberband/include/rubberband/RubberBandStretcher.h"

class PitchShifter{
    public:
        PitchShifter() = default;
        void prepare(double, int);
        void setPitchShiftRatio(float);
        void processBlock(juce::AudioBuffer<float>&);
    private:
        void processRubberBand(int);
        int getAvailableSamples() const;

        double sampleRate;     // Typical sample rates at 44100 or 48000
        int maxBlockSize;
        std::unique_ptr<RubberBand::RubberBandStretcher> stretcher;
        int bufferSize;

        juce::AudioBuffer<float> inputBuffer; // Buffer sizes are mismatched by RubberBand so have two buffers to handle this
        juce::AudioBuffer<float> outputBuffer;

        //indexes
        int writeIn = 0;
        int writeOut = 0;
        int readIn = 0;
        int readOut = 0;

        std::atomic<float> pitchRatio { 1.0f }; // thread safe
};