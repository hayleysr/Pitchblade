#pragma once
#include <vector>
#include <memory>
#include <juce_audio_processors/juce_audio_processors.h>

#include "Pitchblade/effects/GainProcessor.h"       //Austin
#include "Pitchblade/effects/FormantDetector.h"     //huda
#include "Pitchblade/effects/NoiseGateProcessor.h"  //austin
#include "Pitchblade/effects/PitchDetector.h"       //hayley
#include "Pitchblade/effects/CompressorProcessor.h" //Austin

#include "Pitchblade/panels/EffectNode.h"

class EffectNode;

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

	//============================== dsp
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //============================== editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //============================== info
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

	//============================== programs
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

	//============================== state
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

<<<<<<< HEAD
    //GAIN STUFF====================================================================
    float gainDB = 0.0f;

    //NOISE GATE STUFF==============================================================
    float gateThresholdDb = -48.0f;
    float gateAttack = 25.0f;
    float gateRelease = 100.0f;

    // Formant Detector stuff ===================================================
    bool showFormantGUI = false;  // true = Formant view, false = Gain view - huda
    FormantDetector& getFormantDetector() { return formantDetector; }
    const std::vector<float>&getLatestFormants() const {return latestFormants;}

    //COMPRESSOR STUFF==============================================================
    float compressorThresholdDb = -20.0f;
    float compressorRatio = 2.0f;
    float compressorAttack = 10.0f;
    float compressorRelease = 100.0f;
    bool isLimiterMode = 0;

    //UI STUFF====================================================================
    //bypass on/off
    bool isBypassed();
=======
    //============================== global bypass
    bool isBypassed() const;
>>>>>>> main
    void setBypassed(bool newState);
    //============================== stored perameters for daisy chain
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	std::vector<std::shared_ptr<EffectNode>>& getEffectNodes() { return effectNodes; }  //getter for effect nodes

    //============================== DSP processors 

    GainProcessor& getGainProcessor() { return gainProcessor; }
    NoiseGateProcessor& getNoiseGateProcessor() { return noiseGateProcessor; }

    FormantDetector& getFormantDetector() { return formantDetector; }
        void setLatestFormants(const std::vector<float>& freqs) { latestFormants = freqs; }
        const std::vector<float>& getLatestFormants() { return latestFormants; }

    PitchDetector& getPitchDetector() { return pitchProcessor; }
    CompressorProcessor& getCompressorProcessor() { return compressorProcessor; }

private:
    //============================== 
    GainProcessor gainProcessor;            //austin
    NoiseGateProcessor noiseGateProcessor;
    FormantDetector formantDetector;        // To handle detection - huda
    std::vector<float> latestFormants;      // Vector to store formants - huda
    PitchDetector pitchProcessor;           //hayley

    bool bypassed = false;

    CompressorProcessor = compressorProcessor;
    
	// reyna    Effect nodes for the processing chain
    std::vector<std::shared_ptr<EffectNode>> effectNodes;
    std::shared_ptr<EffectNode> rootNode;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};