//Written by Austin Hills

#include "Pitchblade/effects/DeEsserProcessor.h"

DeEsserProcessor::DeEsserProcessor(){}

void DeEsserProcessor::prepare(double sRate, int samplesPerBlock){
    sampleRate = sRate;
    updateAttackAndRelease();
    updateFilter();

    //Prepare the IIR filters with the process spec
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;

    for(auto& filter : sidechainFilters){
        filter.prepare(spec);
        filter.reset();
    }
}

//Setters for user controlled parameters
void DeEsserProcessor::setThreshold(float thresholdDB){
    threshold=thresholdDB;
}

void DeEsserProcessor::setRatio(float ratioValue){
    ratio=ratioValue;
}

void DeEsserProcessor::setAttack(float attackMs){
    attackTime=attackMs;
    updateAttackAndRelease();
}

void DeEsserProcessor::setRelease(float releaseMs){
    releaseTime=releaseMs;
    updateAttackAndRelease();
}

void DeEsserProcessor::setFrequency(float frequencyInHz){
    frequency = frequencyInHz;
    updateFilter();
}

//Calculates the smoothing coefficients for the envelope
void DeEsserProcessor::updateAttackAndRelease(){
    attackCoeff = exp(-1.0f / (0.001f * attackTime * sampleRate + 0.0000001f));
    releaseCoeff = exp(-1.0f / (0.001f * releaseTime * sampleRate + 0.0000001f));
}

//Calculates the coefficients for the sidechain band-pass filter
void DeEsserProcessor::updateFilter(){
    //Create band-pass filter coefficients centered around the target sibilant frequency
    // The Q value of 1.0f is a reasonable starting point, but it might be worthwhile to allow the user to control
    auto coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate,frequency,1.0f);

    //Assign the new coefficients to both stereo filters
    for(auto& filter : sidechainFilters){
        filter.coefficients = *coefficients;
    }
}

//Main processing loop
void DeEsserProcessor::process(juce::AudioBuffer<float>& buffer){
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for(int i = 0;i < numSamples; i++){
        //Find loudest sibilant sample across all channels
        float sidechainSample = 0.0f;
        for(int j = 0; j < numChannels; j++){
            //Get the unfiltered audio sample
            float originalSample = buffer.getSample(j,i);

            //Apply the sidechain filter to isolate sibilant frequencies
            //This does not affect the main audio
            float filteredSample = sidechainFilters[j].processSample(originalSample);

            //Find the peak
            sidechainSample = std::max(sidechainSample, std::abs(filteredSample));
        }

        //Envelope detection. The envelope tracks the loudness of the sibilance
        if(sidechainSample > envelope){
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * sidechainSample;
        }else{
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * sidechainSample;
        }

        //Gain calculation (identical to compressor)
        float gainReductionDB = 0.0f;
        float envelopeDB = juce::Decibels::gainToDecibels(envelope,-100.0f);

        if(envelopeDB > threshold){
            gainReductionDB = (envelopeDB - threshold) * (1.0f - (1.0f / ratio));
        }

        float gainMultiplier = juce::Decibels::decibelsToGain(-gainReductionDB);

        //Apply gain
        for(int j = 0; j < numChannels; j++){
            float* channelData = buffer.getWritePointer(j);
            channelData[i] *= gainMultiplier;
        }
    }
}