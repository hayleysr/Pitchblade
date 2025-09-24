//Written by Austin Hills

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <JuceHeader.h>

//Defining the class that handles the processing of gain
class GainProcessor
{
private:
    //Default gain is set to 1
    float currentGain = 1.0f;

public:
    //Constructor
    GainProcessor();

    //Sets the gain value from the main processor using a user defined value
    void setGain(float gainInDB);

    //Function to process the audio buffer
    void process(juce::AudioBuffer<float>& buffer);

};