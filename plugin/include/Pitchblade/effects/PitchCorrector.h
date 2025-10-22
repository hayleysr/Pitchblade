/**
 * Author: Hayley Spellicy-Ryan
 * 
 * Pitch Correction
 */
#pragma once

#include "PitchDetector.h"
#include "PitchShifter.h"

enum scaleType{
    Major,
    Minor
};

class PitchCorrector{
public:
    PitchCorrector() = default;
    void prepare(double, int);
    void processBlock(juce::AudioBuffer<float>&);
    void setScale(int);
    void setRetuneSpeed(float);
    void setCorrectionRatio(float);

    float getCurrentPitch() { return pitchDetector.getCurrentPitch(); }
    std::string getCurrentNoteName() { return pitchDetector.getCurrentNoteName(); }

private:
    int quantizeToScale(int);
    static float noteToFrequency(int midi);

    PitchDetector pitchDetector;
    PitchShifter pitchShifter;

    std::vector<std::vector<int>> scale = {
        { 12, 14, 16, 17, 19, 21, 23}, // Major
        { 12, 14, 15, 17, 19, 20, 22}  // Minor
    };
    int scaleType = 0;

    float currentRatio = 1.0f;
    float retuneSpeed = 0.1f;
    float correctionRatio = 1.0f;

    double sampleRate;
};