//Austin

#include <gtest/gtest.h>
#include <JuceHeader.h>
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
#include "Pitchblade/PluginProcessor.h"

class IntegrationPerformanceTest : public ::testing::Test {
protected:
    juce::ScopedJuceInitialiser_GUI guiInitialiser;

    std::unique_ptr<AudioPluginAudioProcessor> plugin;
    juce::AudioBuffer<float> buffer;
    juce::MidiBuffer midiBuffer;

    void SetUp() override {
        plugin = std::make_unique<AudioPluginAudioProcessor>();
        plugin->prepareToPlay(44100.0, 512);

        buffer.setSize(2, 512);

        juce::Random r;
        for(int channel = 0; channel < buffer.getNumChannels(); ++channel){
            for(int i = 0; i < buffer.getNumSamples(); ++i){
                buffer.setSample(channel, i, r.nextFloat() * 2.0f - 1.0f);
            }
        }
    }

    void validateEffectPerformance(const std::string& effectName, double maxDurationMs = 20.0){
        plugin->loadDefaultPreset("init");

        std::vector<AudioPluginAudioProcessor::Row> layout = {{effectName, ""}};
        plugin->requestLayout(layout);

        for(int i = 0; i < 3; ++i){
            plugin->processBlock(buffer, midiBuffer);
        }

        // Measure
        double maxTime = 0.0;
        for(int i = 0; i < 5; ++i){
            auto start = std::chrono::high_resolution_clock::now();
            plugin->processBlock(buffer, midiBuffer);
            auto end = std::chrono::high_resolution_clock::now();

            double ms = std::chrono::duration<double, std::milli>(end - start).count();
            if(ms > maxTime) maxTime = ms;
        }

        EXPECT_LT(maxTime, maxDurationMs) << "Performance failure: " << effectName << " took " << maxTime << " ms";
    }
};

//Test for Integration Performance
TEST_F(IntegrationPerformanceTest, AllEffectsUnder20ms){

    std::vector<std::string> effects = {
        "Gain",
        "Noise Gate",
        "Compressor",
        "De-Esser",
        "De-Noiser",
        "Formant",
        "Pitch",
        "Equalizer"
    };

    for(const auto& effect : effects){
        validateEffectPerformance(effect);
    }
}