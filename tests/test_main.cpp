#include <gtest/gtest.h>
#include <JuceHeader.h>

// This sets up JUCE GUI and the MessageManager for the lifetime of the tests
int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInit;  // create MessageManager, timers are now legal

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
