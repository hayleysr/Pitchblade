#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/PluginEditor.h"

#include "Pitchblade/panels/GainPanel.h"
#include "Pitchblade/panels/NoiseGatePanel.h"
#include "Pitchblade/panels/FormantPanel.h"
#include "Pitchblade/panels/PitchPanel.h"
#include "Pitchblade/panels/CompressorPanel.h"
#include "Pitchblade/panels/DeEsserPanel.h"
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
        "GAIN", "Gain", juce::NormalisableRange<float>(-48.0f, 48.0f, 0.1f), 0.0f));

    // Noise gate : austin
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "GATE_THRESHOLD", "Gate Threshold", juce::NormalisableRange<float>(-100.0f, 0.0f, 0.1f), -100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "GATE_ATTACK", "Gate Attack", juce::NormalisableRange<float>(1.0f, 200.0f, 1.0f), 25.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "GATE_RELEASE", "Gate Release", juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f), 100.0f));

    // Compressor : austin
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "COMP_THRESHOLD", "Compressor Threshold", juce::NormalisableRange<float>(-100.0f, 0.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "COMP_RATIO", "Compressor Ratio", juce::NormalisableRange<float>(1.0f, 10.0f, 0.1f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "COMP_ATTACK", "Compressor Attack", juce::NormalisableRange<float>(1.0f, 300.0f, 0.1f), 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "COMP_RELEASE", "Compressor Release", juce::NormalisableRange<float>(1.0f, 300.0f, 0.1f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "COMP_LIMITER_MODE", "Compressor Limiter Mode", "False"));

    //De-Esser : austin
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DEESSER_THRESHOLD", "DeEsser Threshold", juce::NormalisableRange<float>(-100.0f, 0.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DEESSER_RATIO", "DeEsser Ratio", juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f), 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DEESSER_ATTACK", "DeEsser Attack", juce::NormalisableRange<float>(1.0f, 200.0f, 0.1f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DEESSER_RELEASE", "DeEsser Release", juce::NormalisableRange<float>(1.0f, 300.0f, 0.1f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DEESSER_FREQUENCY", "DeEsser Frequency", juce::NormalisableRange<float>(2000.0f, 12000.0f, 10.0f), 6000.0f));

    //Settings Panel: austin
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "GLOBAL_FRAMERATE", "Global Framerate", 1, 4, 3));

    return { params.begin(), params.end() };
}

///// reorder request from UI thread///////////////////////////////////////
// will instead store names of nodes in new order, and then apply reorder on audio thread 
void AudioPluginAudioProcessor::requestReorder(const std::vector<juce::String>& newOrderNames) {
	std::lock_guard<std::recursive_mutex> lock(audioMutex);   //lock mutex for thread safety
	pendingOrderNames = newOrderNames;              //store new order
	reorderRequested.store(true);                   //set flag to apply reorder
}

////// Apply pendingreorder onto audio thread 
//reconnecting of effect nodes based on pending order names - reyna 
void AudioPluginAudioProcessor::applyPendingReorder() {
    std::scoped_lock lock(audioMutex);
	if (!reorderRequested.exchange(false))  //check and reset flag
        return;
	// Debug output of pending rows
    juce::Logger::outputDebugString("============================================");
    juce::Logger::outputDebugString("=== applyPendingReorder called ===");
    for (int i = 0; i < (int)pendingRows.size(); ++i) {
        const auto& r = pendingRows[i];
        juce::Logger::outputDebugString(
            "[" + juce::String(i) + "] Left: " + (r.left.isNotEmpty() ? r.left : "(none)")
            + " | Right: " + (r.right.isNotEmpty() ? r.right : "(none)")
        );
    }
    juce::Logger::outputDebugString("=============");
	// Debug output of current effect nodes and their modes
    juce::Logger::outputDebugString("========== Effect Modes (Nodes) ==========");
    for (int i = 0; i < (int)effectNodes.size(); ++i) {
        if (auto& n = effectNodes[i]) {
            auto modeToStr = [](ChainMode m) {
                switch (m) {
                case ChainMode::Down:       return "down";
                case ChainMode::Split:      return "split";
                case ChainMode::DoubleDown: return "doubleDown";
                case ChainMode::Unite:      return "unite";
                default:                    return "?";
                }
                };
            juce::Logger::outputDebugString( "[" + juce::String(i) + "] " + n->effectName + " | Mode: " + juce::String(modeToStr(n->chainMode))
            );
        }
    }
    juce::Logger::outputDebugString("===========================================");

    //copy of current list
    std::vector<std::shared_ptr<EffectNode>> oldNodes = std::move(effectNodes);
    std::vector<std::shared_ptr<EffectNode>> newList;
    newList.reserve(pendingOrderNames.size());  

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
        if (!n) { continue; 
        }

		// helper lambda to connect if valid index
        auto connectIfValid = [&](int from, int to) {
                if (to >= 0 && to < (int)newList.size()) {
                    auto& src = newList[from];
                    auto& dst = newList[to];
                    if (src && dst) 
                        src->connectTo(dst);
                        juce::Logger::outputDebugString("connect " + src->effectName + " >>> " + dst->effectName);
                    }
            };

		// connect based on chain mode
        switch (n->chainMode) {
            case ChainMode::Down:
                connectIfValid(i, i + 1); 
                juce::Logger::outputDebugString("mode Down : " + n->effectName);
                break;

            case ChainMode::Split:                   // connect to next two nodes
                connectIfValid(i, i + 1);
                connectIfValid(i, i + 2);
                juce::Logger::outputDebugString("mode Split : " + n->effectName);
                break;

            case ChainMode::DoubleDown:             // parallel : connect to next and skip one
                connectIfValid(i, i + 1);
                connectIfValid(i, i + 2);
                juce::Logger::outputDebugString("mode doubleDown : " + n->effectName);
                break;

            case ChainMode::Unite:                  // merge previous two nodes into this one
                if (i - 1 >= 0 && newList[i - 1]) {
                    newList[i - 1]->connectTo(n);
                    juce::Logger::outputDebugString("connect " + newList[i - 1]->effectName + " >>> " + n->effectName);
                }
                if (i - 2 >= 0 && newList[i - 2]) {
                    newList[i - 2]->connectTo(n);
                    juce::Logger::outputDebugString("connect " + newList[i - 2]->effectName + " >>> " + n->effectName);
                }
                juce::Logger::outputDebugString("mode Unite : " + n->effectName);
                break;

            default:
                break;
        }
    }

    // update effect node list
    effectNodes = std::move(newList);
    activeNodes = std::make_shared<std::vector<std::shared_ptr<EffectNode>>>(effectNodes);
	rootNode = !effectNodes.empty() ? effectNodes.front() : nullptr;    //reset root node
    juce::Logger::outputDebugString("=== Reorder finished ===\n");

	////////////////////////////////////debug printout : current effect chain ///////////////////////
    juce::Logger::outputDebugString("============================================");
    juce::Logger::outputDebugString("========== Current Effect Chain ==========");
    for (int i = 0; i < (int)pendingRows.size(); ++i) {
        const auto& row = pendingRows[i];
        juce::String msg;
        msg << "[" << i << "]  Left: " << (row.left.isNotEmpty() ? row.left : "(empty)");

        if (row.right.isNotEmpty()) msg << " | Right: " << row.right;
        else msg << " | Right: (none)";
        juce::Logger::outputDebugString(msg);
    }
    juce::Logger::outputDebugString("========== Effect Modes ==========");
    for (int i = 0; i < (int)effectNodes.size(); ++i) {
        if (auto& n = effectNodes[i]) {
            juce::String mode;
            switch (n->chainMode) {
            case ChainMode::Down:       mode = "down"; break;
            case ChainMode::Split:      mode = "split"; break;
            case ChainMode::DoubleDown: mode = "doubleDown"; break;
            case ChainMode::Unite:      mode = "unite"; break;
            default:                    mode = "unknown"; break;
            }

            juce::Logger::outputDebugString( "[" + juce::String(i) + "] " + n->effectName + " | Mode: " + mode);
        }
    }
    juce::Logger::outputDebugString("===============================================");
}


//==============================================================================
// layout request from UI thread 
// stores new rows and applies on audio thread - reyna
void AudioPluginAudioProcessor::requestLayout(const std::vector<Row>& newRows) {
    std::lock_guard<std::recursive_mutex> lock(audioMutex);
    pendingRows = newRows;
    layoutRequested.store(true);
}

// helper to find node by name in list
static std::shared_ptr<EffectNode> findByName(const std::vector<std::shared_ptr<EffectNode>>& list, const juce::String& name) {
    for (auto& n : list) if (n && n->effectName == name) return n;
    return {};
}

// apply pending layout on audio thread
// reconnect effect nodes based on pending rows
void AudioPluginAudioProcessor::applyPendingLayout() {
	std::scoped_lock lock(audioMutex);   // lock mutex for thread safety
    if (!layoutRequested.exchange(false))
        return;

	std::vector<std::shared_ptr<EffectNode>> old = effectNodes; // copy of current list
    for (auto& n : old) if (n) {
		n->clearConnections(); n->chainMode = ChainMode::Down;  // reset connections and mode
    }

	auto getRowNodes = [&](const Row& r) -> std::pair<std::shared_ptr<EffectNode>, std::shared_ptr<EffectNode>> {   // helper to get nodes by row
        return { findByName(old, r.left), r.right.isNotEmpty() ? findByName(old, r.right) : nullptr };
        };

	std::shared_ptr<EffectNode> prevL = nullptr, prevR = nullptr;   // previous left/right nodes 
    bool prevWasDouble = false;

	for (int i = 0; i < (int)pendingRows.size(); ++i) { // iterate rows
        const auto& r = pendingRows[i];
        auto [L, R] = getRowNodes(r);
        const bool currDouble = (bool)R;
        const bool nextDouble = (i + 1 < (int)pendingRows.size()) && pendingRows[i + 1].right.isNotEmpty();

		// connect based on current/previous row types //////
		if (currDouble) { // if double row
            if (L) L->chainMode = ChainMode::DoubleDown;
            if (R) R->chainMode = ChainMode::DoubleDown;
            // connect continuation from previous double
            if (prevWasDouble) {
                if (prevL && L) prevL->connectTo(L);
                if (prevR && R) prevR->connectTo(R);
            }
            // if previous was single, it must have been a Split
            else if (prevL) {
                prevL->chainMode = ChainMode::Split;
                if (L) prevL->connectTo(L);
                if (R) prevL->connectTo(R);
            }
            prevL = L; prevR = R; prevWasDouble = true;
        }
        else { // single row
            if (prevWasDouble) {
                // this row merges two lanes
                if (L) L->chainMode = ChainMode::Unite;
                if (prevL && L) prevL->connectTo(L);
                if (prevR && L) prevR->connectTo(L);
            } else {
                // serial Down
                if (prevL && L) prevL->connectTo(L);
                if (L) L->chainMode = nextDouble ? ChainMode::Split : ChainMode::Down;
            }
			prevL = L; prevR = nullptr; prevWasDouble = false;  // reset right
        }
    }

	// rebuild new effect node list
    std::vector<std::shared_ptr<EffectNode>> newList;
    newList.reserve(pendingRows.size() * 2);
    for (auto& r : pendingRows) { 	                                                                 // add nodes in order of rows
		if (auto n = findByName(old, r.left)) { newList.push_back(n); }	                            // add left node
        if (!r.right.isEmpty()) {
            if (auto m = findByName(old, r.right)) { newList.push_back(m); }    // add right node if exists
        }
    }

	// update effect node list
    if (!newList.empty()) {
        effectNodes = std::move(newList);
        activeNodes = std::make_shared<std::vector<std::shared_ptr<EffectNode>>>(effectNodes);
        rootNode = effectNodes.front();
    }
}

//==============================================================================

// preset save/load - reyna
void AudioPluginAudioProcessor::savePresetToFile(const juce::File& file) {
	std::lock_guard<std::recursive_mutex> lock(audioMutex);             // lock mutex for thread safety

	// create XML root
    juce::XmlElement presetRoot("PitchbladePreset");
    presetRoot.setAttribute("version", 1.0);

    // save each active node explicitly
    juce::XmlElement* nodes = new juce::XmlElement("EffectNodes");
    for (auto& node : effectNodes) {
        if (!node) continue;

		auto nodeXml = node->toXml(); //effectnode subclass toXml
        if (nodeXml != nullptr)
            nodes->addChildElement(nodeXml.release());
    }
	// add nodes to root
    presetRoot.addChildElement(nodes);

    // also store global params
    juce::XmlElement* globals = new juce::XmlElement("GlobalParameters");
    globals->setAttribute("GLOBAL_FRAMERATE",
        (int)*apvts.getRawParameterValue("GLOBAL_FRAMERATE"));
    presetRoot.addChildElement(globals);

    file.getParentDirectory().createDirectory();
    presetRoot.writeTo(file);
    juce::Logger::outputDebugString("Saved preset to: " + file.getFullPathName());
}

void AudioPluginAudioProcessor::loadPresetFromFile(const juce::File& file) {
	std::lock_guard<std::recursive_mutex> lock(audioMutex);                 // lock mutex for thread safety
	std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(file));  // parse XML from file
    if (!xml) return;
	// load global params
    auto* nodes = xml->getChildByName("EffectNodes");
    if (!nodes) return;

    // clear existing nodes before rebuilding
    effectNodes.clear();

	// load each node
    forEachXmlChildElement(*nodes, nodeXml) {
        auto name = nodeXml->getTagName();
        std::shared_ptr<EffectNode> node;

        if      (name == "GainNode")        node = std::make_shared<GainNode>(*this);
        else if (name == "NoiseGateNode")   node = std::make_shared<NoiseGateNode>(*this);
        else if (name == "CompressorNode")  node = std::make_shared<CompressorNode>(*this);
        else if (name == "DeEsserNode")     node = std::make_shared<DeEsserNode>(*this);
        else continue;

        node->loadFromXml(*nodeXml);
        effectNodes.push_back(node);
    }

    // update ui
    juce::MessageManager::callAsync([this]() {
        if (auto* editor = dynamic_cast<AudioPluginAudioProcessorEditor*>(getActiveEditor())) {
            editor->getDaisyChain().rebuild();
            editor->getEffectPanel().refreshTabs();
            editor->getVisualizer().refreshTabs();
        }
        });

    juce::Logger::outputDebugString("loaded preset from: " + file.getFullPathName());
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
    
    currentBlockSize = samplesPerBlock; // Austin

	//intialize dsp processors
    formantDetector.prepare(sampleRate);                        //Initialization for FormantDetector for real-time processing - huda
    noiseGateProcessor.prepare(sampleRate);                     //Sending the sample rate to the noise gate processor AUSTIN HILLS
    compressorProcessor.prepare(sampleRate);                    //Austin
    deEsserProcessor.prepare(sampleRate, samplesPerBlock);      //Austin
    pitchProcessor.prepare(sampleRate, samplesPerBlock);        //hayley

	//effect node building - reyna
    effectNodes.clear();
    effectNodes.push_back(std::make_shared<GainNode>(*this));
    effectNodes.push_back(std::make_shared<NoiseGateNode>(*this));
    effectNodes.push_back(std::make_shared<CompressorNode>(*this));
    effectNodes.push_back(std::make_shared<DeEsserNode>(*this));
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

    applyPendingLayout();
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
    // juce::ignoreUnused (destData);

	// reyna: use APVTS to save state
	// create an XML representation of our state
    auto xml = apvts.copyState().createXml();
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    // juce::ignoreUnused (data, sizeInBytes);

    //reyna
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml) {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

//==============================================================================

// Default preset loader - reyna
void AudioPluginAudioProcessor::loadDefaultPreset(const juce::String& type) {
    std::lock_guard<std::recursive_mutex> lock(audioMutex);

    juce::Logger::outputDebugString("Loading default preset type: " + type);

	// reset the current effectNodes vector to default
    effectNodes.clear();

    // Add the default effects in their intended order
    effectNodes.push_back(std::make_shared<GainNode>(*this));
    effectNodes.push_back(std::make_shared<NoiseGateNode>(*this));
    effectNodes.push_back(std::make_shared<CompressorNode>(*this));
    effectNodes.push_back(std::make_shared<DeEsserNode>(*this));
    effectNodes.push_back(std::make_shared<FormantNode>(*this));
    effectNodes.push_back(std::make_shared<PitchNode>(*this));

    // connect in order gain > noiseGate > compressor > deEsser > fFormant > pitch
    for (auto& node : effectNodes)
        if (node) node->clearConnections();

    for (int i = 0; i + 1 < (int)effectNodes.size(); ++i)
        effectNodes[i]->connectTo(effectNodes[i + 1]);

    activeNodes = std::make_shared<std::vector<std::shared_ptr<EffectNode>>>(effectNodes);
    rootNode = effectNodes.front();

	// update ui
    juce::MessageManager::callAsync([this]() {
        if (auto* editor = dynamic_cast<AudioPluginAudioProcessorEditor*>(getActiveEditor())) {
            editor->getDaisyChain().rebuild();
            editor->getEffectPanel().refreshTabs();
            editor->getVisualizer().refreshTabs();
        }
        });
    juce::Logger::outputDebugString("default preset loaded ");
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