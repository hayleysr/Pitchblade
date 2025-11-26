/**
 * Author: Hayley Spellicy-Ryan
 */

#include <gtest/gtest.h>
#include <JuceHeader.h>
#include "Pitchblade/effects/PitchDetector.h"
#include <vector>

#define PI 3.141592653589793238

// Test setup; use TEST_F for tests that require auxilary classes
class PitchDetectorTest : public ::testing::Test{
protected:
    void SetUp() override{
        thisSampleRate = 44100.f;
        windowSize = 1024;
        detector = std::make_unique<PitchDetector>(windowSize);
        detector->prepare(thisSampleRate, 512, 4);
    }
    
    //from hayley: Helper to construct individual frame of sine wave
    std::vector<float> makeSineFrame(float frequency, int bufferSize, double thisSampleRate){
        std::vector<float> sineFrame(bufferSize);
        for(int i = 0; i < bufferSize; ++i){
            double timeIndex = double(i) / thisSampleRate;
            sineFrame[i] = float(std::sin(2.0 * PI * frequency * timeIndex));
        }
        return sineFrame;
    }

    double thisSampleRate;
    int windowSize;
    std::unique_ptr<PitchDetector> detector;
};

TEST_F(PitchDetectorTest, DetectEmptyPitch)
{
    // --- 1. ARRANGE ---
    std::vector<float> zeroes(windowSize, 0.f);
    // --- 2. ACT ---
    detector->processFrame(zeroes);
    // --- 3. ASSERT ---
    ASSERT_FLOAT_EQ(detector->getCurrentPitch(), 0.f);
}

TEST_F(PitchDetectorTest, Detect440Hz)
{
    // --- 1. ARRANGE ---
    const float frequency = 440.f; // A4
    auto frame = makeSineFrame(frequency, windowSize, thisSampleRate);
    // --- 2. ACT ---
    detector->processFrame(frame);
    // --- 3. ASSERT ---
    float detected = detector->getCurrentPitch();
    const float tolerance = 5.f;   //highest difference between G# and A
    EXPECT_NEAR(detected, frequency, tolerance);
}

TEST_F(PitchDetectorTest, DetectEmptyMidi)
{
    // --- 1. ARRANGE ---
    std::vector<float> zeroes(windowSize, 0.f);
    // --- 2. ACT ---
    detector->processFrame(zeroes);
    // --- 3. ASSERT ---
    ASSERT_FLOAT_EQ(detector->getCurrentMidiNote(), 0.f);

}

TEST_F(PitchDetectorTest, Detect69Midi)
{
    // --- 1. ARRANGE ---
    const int targetnote = 69; // A4
    const int frequency = 440.f;
    auto frame = makeSineFrame(frequency, windowSize, thisSampleRate);
    // --- 2. ACT ---
    detector->processFrame(frame);
    // --- 3. ASSERT ---
    float detected = detector->getCurrentMidiNote();
    const float tolerance = 0;
    EXPECT_NEAR(detected, targetnote, tolerance);
}

TEST_F(PitchDetectorTest, DetectEmptyNoteName)
{
    // --- 1. ARRANGE ---
    std::vector<float> zeroes(windowSize, 0.f);
    // --- 2. ACT ---
    detector->processFrame(zeroes);
    // --- 3. ASSERT ---
    ASSERT_EQ(detector->getCurrentNoteName(), "A"); 
}

TEST_F(PitchDetectorTest, DetectANoteName)
{
    // --- 1. ARRANGE ---
    const float frequency = 440.f; // A4
    auto frame = makeSineFrame(frequency, windowSize, thisSampleRate);
    // --- 2. ACT ---
    detector->processFrame(frame);
    // --- 3. ASSERT ---
    ASSERT_EQ(detector->getCurrentNoteName(), "A");
}