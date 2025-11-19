#include <gtest/gtest.h>
#include <JuceHeader.h>
#include "Pitchblade/effects/NoiseGateProcessor.h"

// Helper class
class NoiseGateTest : public::testing::Test{
protected:
    NoiseGateProcessor processor;
    juce::AudioBuffer<float> buffer;
    double sampleRate = 44100.0;
    int bufferSize = 512;

    NoiseGateTest()
        : buffer(1, bufferSize){}

    void SetUp() override{
        processor.prepare(sampleRate);
    }

    //Helps to process multiple blocks to simulate time passing
    void processBlocks(int numBlocks){
        for(int i = 0; i < numBlocks; i++){
            processor.process(buffer);
        }
    }

    //Helper to calculate blocks needed for a duration in ms
    int blocksForMS(float ms){
        return (int)(std::ceil((ms/1000) * sampleRate / (double)bufferSize));
    }
};

//Test for TC-05
TEST_F(NoiseGateTest, TC_05){
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        juce::FloatVectorOperations::fill(buffer.getWritePointer(channel), 0.01f, buffer.getNumSamples());
    processor.setThreshold(-30.0f);
    processor.setAttack(10.0f);
    processor.setRelease(10.0f);

    processBlocks(blocksForMS(0.5f));

    ASSERT_NEAR(buffer.getSample(0,100),0.0f,0.001f);
}