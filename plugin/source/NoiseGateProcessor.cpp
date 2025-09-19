//Written by Austin Hills

#include "Pitchblade/NoiseGateProcessor.h"

//Constructor
NoiseGateProcessor::NoiseGateProcessor(){}

//Function called before processing to prepare the gate with the sample rate being used
void NoiseGateProcessor::prepare(const double sRate){
    sampleRate = sRate;
    updateAttackAndRelease();
}

//Setters
void NoiseGateProcessor::setThreshold(float thresholdInDB){
    threshold = juce::Decibels::decibelsToGain(thresholdInDB);
}
void NoiseGateProcessor::setAttack(float attackInMs){
    attackTime = attackInMs;
    updateAttackAndRelease();
}
void NoiseGateProcessor::setRelease(float releaseInMs){
    releaseTime = releaseInMs;
    updateAttackAndRelease();
}

//This function will be used internally upon any changes in attack, release, or sample rate. It helps to make the opening or closing of the gate gradual rather than instant, which would cause popping.
void NoiseGateProcessor::updateAttackAndRelease(){
    //These functions essentially create a value really close to 1, which when using the sample rate, can cause the gate to open or close in the specified time respectively
    //Furthermore, a linear value is not used simply to make it sound better
    //Finally, in case the user selects an attack or release time of 0, a very small number is added on to prevent a crash
    attackCoeff = exp(-1.0f / (0.001f * attackTime * sampleRate + 0.0000001f));
    releaseCoeff = exp(-1.0f / (0.001f * releaseTime * sampleRate + 0.0000001f));
}

//Processes the input buffer
void NoiseGateProcessor::process(juce::AudioBuffer<float>& buffer){
    //First, we look at the buffer to get some info on it, namely the number of samples and channels
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    //Now, the function iterates through the samples, checking the channel with the maximum volume in the given sample
    //That value is stored and then used to determine if the envelope should be opened or closed
    for(int i = 0;i < numSamples; i++){
        float maxSample = 0.0f;
        for(int j = 0;j < numChannels; j++){
            maxSample = std::max(maxSample, abs(buffer.getSample(j, i)));
        }

        //Create a target value for the envelope to try to go toward
        float targetEnvelope = 0.0f;
        
        //If the sample is louder than the threshold, the targetEnvelope is set to the fully open value. Else, it is set to the fully closed value
        if(maxSample > threshold){
            targetEnvelope = 1.0f;
        }else{
            targetEnvelope = 0.0f;
        }

        //If the target for the envelope is greater than the envelope, then the function will gradually open the envelope. Otherwise, it will gradually close the envelope.
        //If you're interested, the attackCoeff is very close to 1, but just under. It then multiplies with the envelope, slightly lowering it. Then, it adds a tiny bit onto it based on the attackCoeff value
        //The same formula is used for the release so that the code can be modified. For instance, someone might want a noise gate that instead lowers volume to 0.2 rather than 0. I'm not sure why they would
        //want that, but the option is there for them.
        if(targetEnvelope > envelope){
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * targetEnvelope;
        }else{
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * targetEnvelope;
        }

        //This final bit finally alters the buffer audio by multiplying its sound by the envelope
        for(int j = 0;j < numChannels; j++){
            float* channelData = buffer.getWritePointer(j);
            channelData[i] *= envelope;
        }
    }
}