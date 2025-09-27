// Written by Austin Hills

#include "Pitchblade/effects/CompressorProcessor.h"

//Constructor
CompressorProcessor::CompressorProcessor(){}

// Prepare the processor with the current sample rate
void CompressorProcessor::prepare(const double sRate){
    sampleRate = sRate;
    updateAttackAndRelease();
}

// Setters for user controlled parameters
void CompressorProcessor::setThreshold(float thresholdInDB){
    thresholdDB = thresholdInDB;
}

// For the ratio, it is assumed that the ratio value will be to 1, meaning that no values below 1 are accepted because those will work in the opposite direction
void CompressorProcessor::setRatio(float ratioValue){
    if(ratioValue < 1.0f){
        ratio = 1.0f;
    }else{
        ratio = ratioValue;
    }
}

void CompressorProcessor::setAttack(float attackInMS){
    attackTime = attackInMS;
    updateAttackAndRelease();
}

void CompressorProcessor::setRelease(float releaseInMS){
    releaseTime = releaseInMS;
    updateAttackAndRelease();
}

// Calculates the smoothing coefficients for the envelope detector
void CompressorProcessor::AttackAndRelease(){
    
}