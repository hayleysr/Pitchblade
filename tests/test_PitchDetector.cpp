/**
 * Author: Hayley Spellicy-Ryan
 */

#include <gtest/gtest.h>
#include <JuceHeader.h>
#include "Pitchblade/effects/PitchDetector.h"

// Test setup
class PitchDetectorTest : public ::testing::Test{
protected:
    void SetUp() override{
        double sampleRate = 44100.f;
        int windowSize = 1024;
        detector = std::make_unique<PitchDetector>(windowSize);
        detector->prepare(sampleRate, 512, 4);
    }

    std::unique_ptr<PitchDetector> detector;
    std::vector<float> zeroes;
};

TEST(PitchDetectorTest, Task1)
{
    // --- 1. ARRANGE ---
    
    // --- 2. ACT ---

    // --- 3. ASSERT ---

}