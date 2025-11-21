#include <gmock/gmock.h>
#include "../plugin/include/Pitchblade/effects/PitchDetector.h"
#include "../plugin/include/Pitchblade/effects/PitchShifter.h"

class MockPitchDetector : public IPitchDetector{
    MOCK_METHOD(void, prepare, (double, int, double), (override));
    MOCK_METHOD(void, processBlock, (const juce::AudioBuffer<float>&), (override));
    MOCK_METHOD(float, getCurrentPitch, (), (override));
    MOCK_METHOD(float, getCurrentMidiNote, (), (override));
};

class MockPitchShifter : public IPitchShifter{
    MOCK_METHOD(void, prepare, (double, int), (override));
    MOCK_METHOD(void, setPitchShiftRatio, (float), (override));
    MOCK_METHOD(void, processBlock, (juce::AudioBuffer<float>&), (override));
};