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
    void setSmoothing(float);

    float getCurrentPitch() { return pitchDetector.getCurrentPitch(); }
    float getTargetPitch() { return targetNote; }
    std::string getCurrentNoteName() { return pitchDetector.getCurrentNoteName(); }
    std::string getTargetNoteName();

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
    float smoothing = 1.0f;

    int targetNote;
    std::string targetNoteName = "A";

    std::string aNoteNames[12] = {
        "A", "A#", "B", "C", "C#", "D", 
        "D#", "E", "F", "F#", "G", "G#"
    };

    double sampleRate;
};