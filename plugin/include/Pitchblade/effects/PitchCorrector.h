/**
 * Author: Hayley Spellicy-Ryan
 * 
 * Pitch Correction
 */
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "PitchDetector.h"
#include "../../third-party/rubberband/include/rubberband/RubberBandStretcher.h"
#include <JuceHeader.h>

enum class CorrectionState{
    Bypass,         // Pass through unchanged
    Transitioning,  // Pitch started from no audio. Slow rampup
    Correcting,     // Fully in pitch correction, main state
    Gliding         // Moving between sung notes, portamento
};

class PitchCorrector{
public:
    PitchCorrector(int, int);
    PitchCorrector();
    ~PitchCorrector();

    void prepare(double, int, double);
    void processBlock(juce::AudioBuffer<float> &);

    void setRetuneSpeed(float);
    void setNoteTransition(float);
    void setNoteTolerance(float);
    void setScale(const std::vector<int>&);
    void setTargetNote(int);

    CorrectionState getCurrentState() const { return dCorrectionState; }
    int getCurrentNote() const { return dCurrentNote; }
    int getTargetNote() const { return dTargetNote; }
    float getCurrentPitch() { return oPitchDetector.getCurrentPitch(); }
    std::string getCurrentNoteName() { return oPitchDetector.getCurrentNoteName(); }

private:
    std::unique_ptr<RubberBand::RubberBandStretcher> pStretcher;
    PitchDetector oPitchDetector;               // pYIN algo

    juce::AudioBuffer<float> dInputBuffer; // Buffer sizes are mismatched by RubberBand so have two buffers to handle this
    juce::AudioBuffer<float> dOutputBuffer;

    // General parameters
    float dSampleRate;                  // best at 44100 or 48000
    int dChannels;
    int dSamplesPerBlock;    
    std::vector<int> dScale;            // vector that stores valid notes in scale           

    // Buffer indexers
    int dInputWritePos;
    int dOutputReadPos;
    int dOutputWritePos;

    // Correction mechanic parameters
    CorrectionState dCorrectionState;
    float dCorrectionRatio;
    float dTargetPitch;
    int dCurrentNote;
    int dTargetNote;
    int dSamplesInState;                // for pitch transition state, tracks how far in transition we are
    int dMinStateSamples;        

    // Correction style parameters
    float dRetuneSpeed;
    float dNoteTransition;
    float dNoteTolerance;
    float dWaver;

    void setStateMachine(float, float);
    void calculatePitchCorrection(float, float);
    int quantizeToScale(float);
    float noteToFrequency(int);
    void updateSmoothingParameters();
    void processRubberBand();
    void retrieveOutputFromRubberBand();
    int getAvailableInputSamples() const;
};