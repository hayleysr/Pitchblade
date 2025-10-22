#include "Pitchblade/effects/PitchCorrector.h"

PitchCorrector::PitchCorrector(int sampleRate = 44100, int channels = 1) : 
    dSampleRate(sampleRate),
    dChannels(channels),
    dSamplesPerBlock(0),
    dInputWritePos(0),
    dOutputReadPos(0),
    dOutputWritePos(0),
    dCorrectionState(CorrectionState::Bypass),
    dCorrectionRatio(1.0f),
    dTargetPitch(440.0),
    dCurrentNote(-1),
    dTargetNote(-1),
    dSamplesInState(0),
    dMinStateSamples(sampleRate / 10),
    dRetuneSpeed (1.0f),
    dNoteTransition(0.1f),
    dNoteTolerance(0.1f),
    dWaver(0.1f)
{

}

PitchCorrector::PitchCorrector() : PitchCorrector(44100, 1)
{

}

PitchCorrector::~PitchCorrector()
{
}

void PitchCorrector::prepare(double sampleRate, int samplesPerBlock, double hopSizeDenominator = 4)
{
    dSampleRate = sampleRate;                   //parameter: how many samples per cycle. 44100KHz default. Stay by that or 44800 for good results
    dSamplesPerBlock = samplesPerBlock;         //parameter: max size that will ever be passed to buffer

    // Options optimized for real time high quality processing
    RubberBand::RubberBandStretcher::Options options = 
        RubberBand::RubberBandStretcher::Option::OptionProcessRealTime|
        RubberBand::RubberBandStretcher::Option::OptionEngineFiner|
        RubberBand::RubberBandStretcher::Option::OptionWindowShort|
        RubberBand::RubberBandStretcher::Option::OptionFormantPreserved|
        RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency;
        
    // Rubber band stretcher setup for the phase vocoder mechanic
    pStretcher = std::make_unique<RubberBand::RubberBandStretcher>(
        sampleRate, 1, options, 1.0, dCorrectionRatio);           //args: sampleRate, channels, Options, initialTimeRatio, initialPitchScale
    pStretcher->setMaxProcessSize(dSamplesPerBlock); //inform rubber band of max size

    // Initialize pitch detector
    oPitchDetector.prepare(sampleRate, samplesPerBlock, hopSizeDenominator);

    // Indexers
    dInputBuffer.setSize(1, std::max(dSamplesPerBlock * 8, 8192)); //one channel of max size
    dOutputBuffer.setSize(1, std::max(dSamplesPerBlock * 8, 8192));
    dInputBuffer.clear();
    dOutputBuffer.clear();

    dInputWritePos = 0;
    dOutputReadPos = 0;
    dOutputWritePos = 0;

    // Other parameters
    dCorrectionState = CorrectionState::Bypass;
    dCorrectionRatio = 1.0f;
    dTargetPitch = 220.0f;
    dCurrentNote = -1;
    dTargetNote = 57; //A3
    dSamplesInState = 0;
    dMinStateSamples = dSampleRate / 10;
    dRetuneSpeed = 1.0f;
    dNoteTransition = 0.1f;
    dNoteTolerance = 0.1f;
    dWaver = 0.1f;

    // Set up scale
    dScale.clear();
    for(int i = 0; i < 128; ++i){ // 88-key piano
        dScale.push_back(i);
    }

}
void PitchCorrector::processBlock(juce::AudioBuffer<float> &buffer)
{
    auto bufferNumSamples = buffer.getNumSamples(); 

    juce::AudioBuffer<float> monoBuffer(1, bufferNumSamples);
    if (buffer.getNumChannels() >= 2) {
        auto* left = buffer.getReadPointer(0);
        auto* right = buffer.getReadPointer(1);
        auto* mono = monoBuffer.getWritePointer(0);
        for (int i = 0; i < bufferNumSamples; ++i) {
            mono[i] = 0.5f * (left[i] + right[i]);
        }
    } else {
        monoBuffer.copyFrom(0, 0, buffer, 0, 0, bufferNumSamples);
    }

    // Pitch detection
    oPitchDetector.processBlock(monoBuffer);
    float dDetectedPitch = oPitchDetector.getCurrentPitch();
    
    // Throw out unreasonable values
    if (dDetectedPitch < 50.0f || dDetectedPitch > 2000.0f){
        dDetectedPitch = 0.0f;
    }
    
    float dDetectedNote = oPitchDetector.getCurrentMidiNote();

    // State machine and correction calculation
    setStateMachine(dDetectedPitch, dDetectedNote);
    calculatePitchCorrection(dDetectedPitch, dDetectedNote);

    // Accumulate input samples (mono)
    auto* monoData = monoBuffer.getReadPointer(0);
    for (int i = 0; i < bufferNumSamples; ++i){
        if (dInputWritePos < dInputBuffer.getNumSamples())
        {
            dInputBuffer.setSample(0, dInputWritePos, monoData[i]);
            dInputWritePos++;
        }
    }
    
    DBG("Accumulated: " << dInputWritePos << " | Required: " << pStretcher->getSamplesRequired());
    processRubberBand();

    // Output processed audio to both channels
    for (int channel = 0; channel < buffer.getNumChannels(); channel++){
        auto* outputData = buffer.getWritePointer(channel);
        int samplesCopied = 0;
        
        while (dOutputReadPos != dOutputWritePos && samplesCopied < bufferNumSamples){
            outputData[samplesCopied] = dOutputBuffer.getSample(0, dOutputReadPos);
            dOutputReadPos = (dOutputReadPos + 1) % dOutputBuffer.getNumSamples();
            samplesCopied++;
        }
        
        // Fill undershoots with empty noise.
        for (int i = samplesCopied; i < bufferNumSamples; ++i) {
            outputData[i] = 0.0f;
        }
    }
    
    dSamplesInState += bufferNumSamples;
}

/* parameters / dials */
void PitchCorrector::setRetuneSpeed(float speed)
{ 
    dRetuneSpeed = juce::jlimit(0.0f, 1.0f, speed); // clamp to 0-1
    updateSmoothingParameters();
}
void PitchCorrector::setNoteTransition(float speed)
{ 
    dNoteTransition = juce::jlimit(0.0f, 1.0f, speed);  // clamp to 0-1
    updateSmoothingParameters();
}
void PitchCorrector::setNoteTolerance(float tolerance)
{ 
    dNoteTolerance = juce::jlimit(0.1f, 1.0f, tolerance); 
}
void PitchCorrector::setScale(const std::vector<int>& scale) 
{ 
    dScale = scale; 
    dCurrentNote = -1;
}
void PitchCorrector::setTargetNote(int midiNote) 
{ 
    if (midiNote != dTargetNote) 
    {
        dTargetNote = midiNote;
        dTargetPitch = noteToFrequency(midiNote);
        
        if (dCurrentNote != -1 && dCurrentNote != dTargetNote)
        {
            dCorrectionState = CorrectionState::Transitioning;
            dSamplesInState = 0;
        }
    }
}

void PitchCorrector::setStateMachine(float detectedPitch, float detectedNote)
{
    bool hasPitch = detectedPitch > 0.0f;
    int roundedNote = std::round(detectedNote);
    
    // Check if detected note is valid: in scale, in tolerance band
    bool validNote = hasPitch && 
                    std::find(dScale.begin(), dScale.end(), roundedNote) != dScale.end() &&
                    std::abs(detectedNote - roundedNote) < dNoteTolerance;
    
    switch (dCorrectionState)
    {
    case CorrectionState::Bypass:
        if (validNote && dTargetNote != -1)
        {
            dCorrectionState = CorrectionState::Transitioning;
            dSamplesInState = 0;
            dCurrentNote = roundedNote;
        }
        break;
        
    case CorrectionState::Transitioning:
        if (!hasPitch)
        {
            dCorrectionState = CorrectionState::Bypass;
            dSamplesInState = 0;
        }
        else if (dSamplesInState > dMinStateSamples && validNote)
        {
            dCorrectionState = CorrectionState::Correcting;
            dSamplesInState = 0;
            dCurrentNote = roundedNote;
        }
        break;
        
    case CorrectionState::Correcting:
        if (!hasPitch)
        {
            dCorrectionState = CorrectionState::Bypass;
            dSamplesInState = 0;
        }
        else if (validNote && roundedNote != dCurrentNote)
        {
            // Note change detected
            if (dNoteTransition > 0.8f) // Fast transitions = immediate
            {
                dCurrentNote = roundedNote;
            }
            else // Smooth transitions
            {
                dCorrectionState = CorrectionState::Gliding;
                dSamplesInState = 0;
            }
        }
        break;
        
    case CorrectionState::Gliding:
        if (!hasPitch)
        {
            dCorrectionState = CorrectionState::Bypass;
            dSamplesInState = 0;
        }
        else if (dSamplesInState > dMinStateSamples * (1.0f - dNoteTransition))
        {
            dCorrectionState = CorrectionState::Correcting;
            dSamplesInState = 0;
            dCurrentNote = roundedNote;
        }
        break;
    }
    DBG("=== STATE MACHINE DEBUG ===");
    DBG("Detected Pitch: " << detectedPitch << " Hz");
    DBG("Detected Note: " << detectedNote << " (rounded: " << roundedNote << ")");
    DBG("Has Pitch: " << (hasPitch ? "YES" : "NO"));
    DBG("Valid Note: " << (validNote ? "YES" : "NO"));
    DBG("Target Note: " << dTargetNote);
    DBG("Current State: " << (int)dCorrectionState);
    DBG("Note Tolerance: " << dNoteTolerance);
    DBG("Scale size: " << dScale.size());

    if (hasPitch) {
        float calculatedNote = 69 + 12 * std::log2(detectedPitch / 440.0f);
        DBG("Pitch: " << detectedPitch << " Hz -> MIDI: " << detectedNote << " | Calculated: " << calculatedNote);
    }
}

void PitchCorrector::calculatePitchCorrection(float detectedPitch, float detectedNote)
{
    float targetRatio = 1.0f;
    
    switch (dCorrectionState)
    {
    case CorrectionState::Bypass:
        targetRatio = 1.0f; // No correction
        break;
        
    case CorrectionState::Transitioning:
        {
            // Smoothly introduce correction
            float transitionProgress = juce::jmap(
                static_cast<float>(dSamplesInState), 
                0.0f, static_cast<float>(dMinStateSamples), 
                0.0f, 1.0f);
            
            float targetPitch = dTargetPitch;
            if (dTargetNote == -1 && std::round(detectedNote) != -1)
            {
                // Quantize to nearest scale note if no target set
                targetPitch = noteToFrequency(std::round(detectedNote));
            }
            
            float desiredRatio = targetPitch / detectedPitch;
            targetRatio = juce::jmap(transitionProgress, 1.0f, desiredRatio);
        }
        break;
        
    case CorrectionState::Correcting:
        {
            float targetPitch = dTargetPitch;
            if (dTargetNote == -1)
            {
                // Auto-tune mode: quantize to nearest scale note
                targetPitch = noteToFrequency(quantizeToScale(detectedNote));
            }
            if(detectedPitch <= 0.0f) return;
            targetRatio = targetPitch / detectedPitch;
            
            // Apply correction speed smoothing
            float smoothFactor = juce::jmap(dRetuneSpeed, 0.0f, 1.0f, 0.01f, 0.3f);
            dCorrectionRatio = dCorrectionRatio * (1.0f - smoothFactor) + 
                                targetRatio * smoothFactor;
        }
        break;
        
    case CorrectionState::Gliding:
        {
            // Smooth glide between notes
            float startPitch = noteToFrequency(dCurrentNote);
            float endPitch = noteToFrequency(quantizeToScale(detectedNote));
            
            float glideProgress = juce::jmap(
                static_cast<float>(dSamplesInState), 
                0.0f, static_cast<float>(dMinStateSamples) * (1.0f - dNoteTransition), 
                0.0f, 1.0f);
            
            float interpolatedPitch = juce::jmap(glideProgress, startPitch, endPitch);
            targetRatio = interpolatedPitch / detectedPitch;
        }
        break;
    }
    
    // Apply limits and set RubberBand
    targetRatio = juce::jlimit(0.5f, 2.0f, targetRatio);
    
    // Final smoothing based on state
    float finalWaver = dWaver;
    if (dCorrectionState == CorrectionState::Gliding)
        finalWaver *= 0.5f; // Less smoothing during glides
    
    dCorrectionRatio = dCorrectionRatio * (1.0f - finalWaver) + 
                        targetRatio * finalWaver;
    
    DBG("Pitch Correction - Detected: " << detectedPitch << " Hz, Target: " << dTargetPitch << " Hz, Ratio: " << dCorrectionRatio);
    pStretcher->setPitchScale(dCorrectionRatio);
    pStretcher->setTimeRatio(1.0 / dCorrectionRatio);
}

int PitchCorrector::quantizeToScale(float detectedNote)
{
    if (dScale.empty()) return std::round(detectedNote);
    
    int roundedNote = std::round(detectedNote);
    auto it = std::min_element(dScale.begin(), dScale.end(),
        [roundedNote](int a, int b) {
            return std::abs(a - roundedNote) < std::abs(b - roundedNote);
        });
    
    return (it != dScale.end()) ? *it : roundedNote;
}

float PitchCorrector::noteToFrequency(int noteNumber)
{
    return 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);
}

void PitchCorrector::updateSmoothingParameters()
{
    // Map correction speed to smoothing factor (inverse relationship)
    dWaver = juce::jmap(dRetuneSpeed, 0.0f, 1.0f, 0.05f, 0.005f);
}

void PitchCorrector::processRubberBand()
{
    const int availableInput = getAvailableInputSamples();
    const int requiredInput = pStretcher->getSamplesRequired();
    
    DBG("RubberBand - Available: " << availableInput << " | Required: " << requiredInput);
    
    if (availableInput >= requiredInput && requiredInput > 0)
    {
        
        DBG("RubberBand processing " << requiredInput << " samples");
        std::vector<float*> inputPointers(1);
        std::vector<float> inputData(requiredInput);
        
        // Read from the start of the accumulated buffer
        for (int i = 0; i < requiredInput; ++i)
        {
            inputData[i] = dInputBuffer.getSample(0, i);
        }

        inputPointers[0] = inputData.data();
        pStretcher->process(inputPointers.data(), requiredInput, false);
        
        // Remove processed samples, shift remaining to front
        int remainingSamples = availableInput - requiredInput;
        for (int i = 0; i < remainingSamples; ++i)
        {
            dInputBuffer.setSample(0, i, dInputBuffer.getSample(0, i + requiredInput));
        }
        dInputWritePos = remainingSamples;
        
        DBG("Processed " << requiredInput << " samples, " << remainingSamples << " remaining in buffer");

        retrieveOutputFromRubberBand();
    }else{
        
        DBG("RubberBand NOT processing - insufficient samples");
    }
}

void PitchCorrector::retrieveOutputFromRubberBand()
{
    const int availableOutput = pStretcher->available();
    
    if (availableOutput > 0)
    {
        std::vector<float*> outputPointers(1);
        std::vector<float> outputData(availableOutput);
        outputPointers[0] = outputData.data();
        
        const int retrieved = pStretcher->retrieve(outputPointers.data(), availableOutput);
        
        for (int i = 0; i < retrieved; ++i)
        {
            dOutputBuffer.setSample(0, dOutputWritePos, outputData[i]);
            dOutputWritePos = (dOutputWritePos + 1) % dOutputBuffer.getNumSamples();
        }
    }
}

int PitchCorrector::getAvailableInputSamples() const
{
    return dInputWritePos;
}
