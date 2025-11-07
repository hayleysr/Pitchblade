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
    float getTargetPitch() { return targetPitch; }
    float getSemitoneError();
    std::string getCurrentNoteName();
    std::string getTargetNoteName();

private:
    int quantizeToScale(int);
    static float noteToFrequency(int midi);
    static float frequencyToNote(int freq);
    float applyParameters(float &midi);

    PitchDetector pitchDetector;
    PitchShifter pitchShifter;

    juce::AudioBuffer<float> monoBuffer;

    std::vector<std::vector<int>> scale = {
        { 12, 14, 16, 17, 19, 21, 23}, // Major
        { 12, 14, 15, 17, 19, 20, 22}  // Minor
    };
    int scaleType = 0;

    // Parameters
    float currentRatio = 1.0f;
    float noteTransition;
    float retuneSpeed;
    float waver;
    //Parameters helpers
    float smoothing = 1.0f;
    float waverPhase = 0.f;
    float prevMidi;
    float lastStableMidi;

    // Hz
    float targetPitch;
    float correctedPitch;

    // Midi
    float currentMidi;
    float targetMidi;
    float semitoneErrorMidi;
    float correctedMidi;


    
    // Note name params
    std::string targetNoteName = "C";
    std::string aNoteNames[12] = {
        "C", "C#", "D", "D#", "E", "F", 
        "F#", "G", "G#", "A", "A#", "B"
    };

    double sampleRate;
};