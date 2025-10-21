#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/PluginEditor.h"

#include "Pitchblade/panels/GainPanel.h"
#include "Pitchblade/panels/NoiseGatePanel.h"
#include "Pitchblade/panels/FormantPanel.h"
#include "Pitchblade/panels/PitchPanel.h"
#include "Pitchblade/panels/CompressorPanel.h"
#include "Pitchblade/panels/EffectNode.h"


//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), 

	//apvts contructor: attachs this to processor
    // AudioProcessorValueTreeState used to managea ValueTree that is used to manage an AudioProcessor's entire state
    apvts(*this, nullptr, "Parameters", createParameterLayout()) {
	        // check if effectNodes tree exists
        if (!apvts.state.hasType("EffectNodes")) {
            apvts.state = juce::ValueTree("EffectNodes");
        }
    }

AudioPluginAudioProcessor::~AudioPluginAudioProcessor(){}

//============================================================================== reyna
// ui stuff: global APVTS perameter layout 
// no longer shared controls works as a template
// defines all default perameters, all effects creates local copies into their valuetree
juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Gain : austin
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "GAIN", "Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));

    // Noise gate : austin
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "GATE_THRESHOLD", "Gate Threshold", juce::NormalisableRange<float>(-80.0f, 0.0f, 0.1f), -48.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "GATE_ATTACK", "Gate Attack", juce::NormalisableRange<float>(1.0f, 200.0f, 1.0f), 25.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "GATE_RELEASE", "Gate Release", juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f), 100.0f));

    // Compressor : austin
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "COMP_THRESHOLD", "Compressor Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "COMP_RATIO", "Compressor Ratio", juce::NormalisableRange<float>(1.0f, 10.0f, 0.1f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "COMP_ATTACK", "Compressor Attack", juce::NormalisableRange<float>(1.0f, 300.0f, 0.1f), 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "COMP_RELEASE", "Compressor Release", juce::NormalisableRange<float>(1.0f, 300.0f, 0.1f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "COMP_LIMITER_MODE", "Compressor Limiter Mode", "False"));

    return { params.begin(), params.end() };
}

///// reorder request from UI thread
// will instead store names of nodes in new order, and then apply reorder on audio thread 
void AudioPluginAudioProcessor::requestReorder(const std::vector<juce::String>& newOrderNames) {
	std::lock_guard<std::mutex> lock(audioMutex);   //lock mutex for thread safety
	pendingOrderNames = newOrderNames;              //store new order
	reorderRequested.store(true);                   //set flag to apply reorder
    applyPendingReorder();
}

////// Apply pendingreorder onto audio thread ////////////////
//reconnecting of effect nodes based on pending order names - reyna 
void AudioPluginAudioProcessor::applyPendingReorder() {
	if (!reorderRequested.exchange(false))  //check and reset flag
        return;

    juce::Logger::outputDebugString("=== [applyPendingReorder called] ===");

    //copy of current list
    std::vector<std::shared_ptr<EffectNode>> oldNodes = std::move(effectNodes);
    std::vector<std::shared_ptr<EffectNode>> newList;

	// rebuild new list based on pending order names
    for (const auto& name : pendingOrderNames) {
        for (auto& n : oldNodes) {
            if (n && n->effectName == name) {
                newList.push_back(n);
                break;
            }
        }
    }

    if (newList.empty()) {    //if no valid names found, do nothing
        juce::Logger::outputDebugString("No valid nodes found to reorder.");
        return;
    }

	//clear all connections in old and new lists > causing stack overflow crash
    for (auto& n : newList) {
        if (n) {
            n->clearConnections();
        }
        juce::Logger::outputDebugString("Rebuilding chain connections safely...");
    }

	//reconnect nodes in new order using daisychain modes
    for (int i = 0; i + 1 < (int)newList.size(); ++i) {
        auto& n = newList[i];
        if (!n) { continue; }
		// connect based on chain mode
        switch (n->chainMode) {
            case ChainMode::Down:
				if (i + 1 < newList.size()) {       // default : connect to next node
                    n->connectTo(newList[i + 1]);
                } break;

            case ChainMode::Split:                   // connect to next two nodes
                if (i + 1 < newList.size()) {   
                    n->connectTo(newList[i + 1]);
                }
                if (i + 2 < newList.size()) {
                    n->connectTo(newList[i + 2]);
                } break;

            case ChainMode::DoubleDown:             // parallel : connect to next and skip one
                if (i + 1 < newList.size()) {
                    n->connectTo(newList[i + 1]);
                }
                if (i + 2 < newList.size()) {
                    n->connectTo(newList[i + 2]);
                } break;

            case ChainMode::Unite:                  // merge previous two nodes into this one
                if (i >= 1 && i < (int)newList.size()) {
                    newList[i - 1]->connectTo(newList[i]);
                }
                if (i >= 2 && i < (int)newList.size()) {
                    newList[i - 2]->connectTo(newList[i]);
				} break;

            default:
                break;
        }
    }

    // update effect node list
    effectNodes = std::move(newList);
    activeNodes = std::make_shared<std::vector<std::shared_ptr<EffectNode>>>(effectNodes);
	rootNode = !effectNodes.empty() ? effectNodes.front() : nullptr;    //reset root node

	////////////////////////////////////debug printout : current effect chain
    juce::Logger::outputDebugString("========== [Current Effect Chain] ==========");
    for (int i = 0; i < (int)effectNodes.size(); ++i) {
        auto& n = effectNodes[i];   
        if (!n) continue;

        juce::String modeName;
        switch (n->chainMode) {
            case ChainMode::Down: modeName      = "Down"; break;
            case ChainMode::Split: modeName     = "Split"; break;
            case ChainMode::DoubleDown: modeName = "DoubleDown"; break;
            case ChainMode::Unite: modeName     = "Unite"; break;
        }

        juce::String msg = "[" + juce::String(i) + "] " + n->effectName + " | Mode: " + modeName + " | Children: ";
        for (auto& c : n->children) {
            if (c) msg += c->effectName + " ";
        }
        juce::Logger::outputDebugString(msg);
    }
    juce::Logger::outputDebugString("============================================");
}

//==============================================================================

const juce::String AudioPluginAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram() {
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index) {
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index) {
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName) {
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
    
	//intialize dsp processors
    formantDetector.prepare(sampleRate);                        //Initialization for FormantDetector for real-time processing - huda
    noiseGateProcessor.prepare(sampleRate);                     //Sending the sample rate to the noise gate processor AUSTIN HILLS
    compressorProcessor.prepare(sampleRate);                    //Austin
    pitchProcessor.prepare(sampleRate, samplesPerBlock, 4);     //hayley

	//effect node building - reyna
    effectNodes.clear();
    effectNodes.push_back(std::make_shared<GainNode>(*this));
    effectNodes.push_back(std::make_shared<NoiseGateNode>(*this));
    effectNodes.push_back(std::make_shared<CompressorNode>(*this));
    effectNodes.push_back(std::make_shared<FormantNode>(*this));
    effectNodes.push_back(std::make_shared<PitchNode>(*this));

	// set up default chain: Gain > Noise gate > formant > Pitch
	for (auto& n : effectNodes) if (n) n->clearConnections();   //clear any existing connections
    for (int i = 0; i + 1 < effectNodes.size(); ++i) {
        effectNodes[i]->connectTo(effectNodes[i + 1]);
    }
	activeNodes = std::make_shared<std::vector<std::shared_ptr<EffectNode>>>(effectNodes);  // shared pointer to active nodes for audio thread
    rootNode = effectNodes.front();

}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
//==============================================================================
void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {   
    //real time processor to update everything
    
    // MOVED ALL PROCESS BLOCK STUFF INTO EFFECTNODE AGAINNN DX - reyna
	// all individual processors are called in their respective effect nodes (in their panel.h)
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

	// reordering chain if requested - reyna
    applyPendingReorder();

    // indivdual bypass checker reyna
	// process and forward only if root node exists and plugin is not globally bypassed
    //  for future chaining effects theyre all the same rn
    if (!isBypassed() && activeNodes && !activeNodes->empty()) {
		auto chain = activeNodes; // copy shared
		auto root = chain->front(); //  get root node
        if (root)
            root->processAndForward(*this, buffer);
    } else {
        buffer.clear();
    }

    //juce boilerplate
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
// looks like save/load stuff that we can use later - reyna
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

//==============================================================================
//Entire plugin bypass functionality - Austin
bool AudioPluginAudioProcessor::isBypassed() const
{
    return bypassed;
}

void AudioPluginAudioProcessor::setBypassed(bool newState)
{
    bypassed = newState;
}