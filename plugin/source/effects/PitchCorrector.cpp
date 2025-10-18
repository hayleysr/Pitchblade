#include "Pitchblade/effects/PitchCorrector.h"

PitchCorrector::PitchCorrector(int sampleRate = 44100, int channels = 1) : 
    dSampleRate(sampleRate),
    dChannels(channels)
{

}

PitchCorrector::PitchCorrector() : PitchCorrector(44100, 1)
{

}

PitchCorrector::~PitchCorrector()
{
    delete pStretcher;
}

void PitchCorrector::prepare(double sampleRate, int samplesPerBlock, double hopSizeDenominator = 4)
{
    dSampleRate = sampleRate;
    dSamplesPerBlock = samplesPerBlock;

    // Options optimized for real time high quality processing
    RubberBand::RubberBandStretcher::Options options = 
        RubberBand::RubberBandStretcher::Option::OptionProcessRealTime|
        RubberBand::RubberBandStretcher::Option::OptionEngineFiner|
        RubberBand::RubberBandStretcher::Option::OptionWindowShort|
        RubberBand::RubberBandStretcher::Option::OptionFormantPreserved|
        RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency;
        
    pStretcher = std::make_unique<RubberBand::RubberBandStretcher>(
        sampleRate, 1, options, 1.0, 1.0);
    
}

void PitchCorrector::processBlock(const juce::AudioBuffer<float> &buffer)
{

}