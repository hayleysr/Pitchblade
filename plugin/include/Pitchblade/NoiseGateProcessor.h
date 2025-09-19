//Written by Austin Hills

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

//Defining the class that handles the simple noise gate
class NoiseGateProcessor
{
private:
    //Threshold refers to the point in linear gain where the sound will be cut off if it dips below or turned on if it goes above.
    float threshold = 0.0f;
    //Attack time refers to the amount of milliseconds before the gate will be opened after the audio goes above the threshold volume. This manifests in a fade rather than an abrupt open
    float attackTime = 50.0f;
    //Release time refers to the amount of milliseconds before the gate will be closed after the audio dips below the threshold volume. This manifests in a fade rather than an abrupt close
    float releaseTime = 50.0f;
    //The following two variables are internal parameters that determine the speed at which the gate's current state changes
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    //The envelope is the current state of the gate, where 0.0 is closed and 1.0 is open
    float envelope = 0.0f;

    //The sample rate will help to convert the unit in milliseconds to something that translates more closely to the audio signal
    double sampleRate = 44100.0;

    //This function will be used internally upon any changes in attack, release, or sample rate. It helps to make a smooth curve for the coefficients to ensure that no clicking or popping happens as the gate's state updates
    void updateAttackAndRelease();

public:
    //constructor
    NoiseGateProcessor();

    //Function called before processing to prepare the gate with the sample rate being used
    void prepare(const double sRate);

    //Setters
    void setThreshold(float thresholdInDB);
    void setAttack(float attackInMs);
    void setRelease(float releaseInMs);

    //Processes the input buffer
    void process(juce::AudioBuffer<float>& buffer);
};