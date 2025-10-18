/**
 * Author: Hayley Spellicy-Ryan
 * 
 * Pitch Correction
 */
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "PitchDetector.h"
#include "rubberband/RubberBandStretcher.h"
#include <JuceHeader.h>

class PitchCorrector{
public:
    PitchCorrector(int, int);
    PitchCorrector();

    void prepare(double, int, double);
    void processBlock(const juce::AudioBuffer<float> &);

private:
    
    //RubberBand::RubberBandStretcher *pStretcher; // Actual pitch shifter
    std::unique_ptr<RubberBand::RubberBandStretcher> pStretcher;
    PitchDetector oPitchDetector;               // pYIN algo

    juce::AudioBuffer<float> dInputBuffer; // Buffer sizes are mismatched by RubberBand so have two buffers to handle this
    juce::AudioBuffer<float> dOutputBuffer;

    float dSampleRate;                  // best at 44100 or 48000
    int dChannels;
    int dSamplesPerBlock;               //

};