#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Pitchblade/GainProcessor.h"
#include "Pitchblade/NoiseGateProcessor.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //GAIN STUFF====================================================================
    float gainDB = 0.0f;

    //NOISE GATE STUFF==============================================================
    float gateThresholdDb = -48.0f;
    float gateAttack = 25.0f;
    float gateRelease = 100.0f;

    //UI STUFF====================================================================
    //bypass on/off
    bool isBypassed();
    void setBypassed(bool newState);

private:
    //==============================================================================
    GainProcessor gainProcessor;
    NoiseGateProcessor noiseGateProcessor;
    bool bypassed = false;
    


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};