#pragma once
#include <vector>
#include <memory>
#include <juce_audio_processors/juce_audio_processors.h>

#include "Pitchblade/effects/GainProcessor.h"       //Austin
#include "Pitchblade/effects/FormantDetector.h"     //huda
#include "Pitchblade/effects/NoiseGateProcessor.h"  //austin
#include "Pitchblade/effects/PitchCorrector.h"      //hayley
#include "Pitchblade/effects/CompressorProcessor.h" //Austin
#include "Pitchblade/effects/DeEsserProcessor.h"    //Austin
#include "Pitchblade/panels/EffectNode.h"           //reyna

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

    //============================== global bypass
    bool isBypassed() const;
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

    PitchCorrector& getPitchCorrector() { return pitchProcessor; }
    CompressorProcessor& getCompressorProcessor() { return compressorProcessor; }
    DeEsserProcessor& getDeEsserProcessor() {return deEsserProcessor; }

    //reyna 
	void requestReorder(const std::vector<juce::String>& newOrderNames);    // reorder using effect names
	void setRootNode(std::shared_ptr<EffectNode> node) { rootNode = std::move(node); }  // set root node for processing chain

    int getCurrentBlockSize() const {return currentBlockSize;}; // Austin - Was having an issue initializing de-esser

	struct Row { juce::String left, right; };               // processing chain row
	void requestLayout(const std::vector<Row>& newRows);    // request new layout for processing chain 

private:
    //============================== 
    //processors
    GainProcessor gainProcessor;            //austin
    NoiseGateProcessor noiseGateProcessor;
    FormantDetector formantDetector;        // To handle detection - huda
    std::vector<float> latestFormants;      // Vector to store formants - huda
    PitchCorrector pitchProcessor;          // To correct pitch - hayley

    bool bypassed = false;

    int currentBlockSize = 512; // Austin

    CompressorProcessor compressorProcessor; //Austin
    DeEsserProcessor deEsserProcessor;      //Austin
    
	// reyna    Effect nodes for the processing chain
    std::vector<std::shared_ptr<EffectNode>> effectNodes;
	std::shared_ptr<std::vector<std::shared_ptr<EffectNode>>> activeNodes; // copy of current active nodes for reordering
    std::shared_ptr<EffectNode> rootNode;

    //reorder queue
	std::mutex audioMutex;                           
	std::atomic<bool> reorderRequested{ false };        // flag for reorder request
	std::vector<juce::String> pendingOrderNames;        // new order to apply

	//layout  rows
	std::vector<Row> pendingRows;   // new layout to apply
    std::atomic<bool> layoutRequested{ false };
    void applyPendingLayout();

    void applyPendingReorder();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};