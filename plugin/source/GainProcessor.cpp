#include "Pitchblade/GainProcessor.h"

//Constructor
GainProcessor::GainProcessor(){}

//Function to set the gain. This converts the decibel value to a linear value for multiplication purposes
void GainProcessor::setGain(float gainInDB){
    //Convert decibels to a linear value
    currentGain = juce::Decibels::decibelsToGain(gainInDB);
}

//Processing the audio buffer to apply the gain
void GainProcessor::process(juce::AudioBuffer<float>& buffer){
    //The following function multiplies the sample in the buffer with the gain value
    buffer.applyGain(currentGain);
}