/**
 * Author: Hayley Spellicy-Ryan
 * PitchDetector class
 * -------------------
 * Fundamental frequency detector for monophonic audio. 
 * Uses the pYIN algorithm.
 */

 #pragma once
 #include <juce_audio_basics/juce_audio_basics.h>
 #include <juce_audio_devices/juce_audio_devices.h>
 #include <juce_dsp/juce_dsp.h> 
 #include <cmath>
 
 class PitchDetector{
    public:
        // Constructor
        PitchDetector(int, float);
        PitchDetector(int);
        PitchDetector();

        // Prepare detector
        void prepare(double, int, double);

        // Process a block of audio
        void processBlock(const juce::AudioBuffer<float>&);

        void processFrame(const std::vector<float>&);

        float getCurrentPitch();
        float getSemitoneError();
        float getCurrentNote();
        std::string getCurrentNoteName();

        // Destructor
        ~PitchDetector();

    private:
        /**
         * Autocorrection, Difference, Cumulative
         */
        void difference(const std::vector<float>&);
        void cumulative();
        int absoluteThreshold();
        float calculateRMS(const std::vector<float>&);

        float dCurrentPitch;                // Pitch of most recent sample batch in Hz
        double dSampleRate;                 // Sample rate
        int dWindowSize;                    // interval i to 2W
        int dYinBufferSize;                 // W, on sum j = t + 1 to t + W
        int dLag;                           // Lag
        std::vector<float> dYinBuffer;      // YIN buffer
        std::vector<float> dCircularBuffer; // Accumulative buffer of samples from AudioBuffer
        int dCircularIdx;                   // Start position in circular buffer
        int dHopSize;                       // Amount to jump fwd by. Creates overlapping frames.
        juce::dsp::WindowingFunction<float>::WindowingMethod dWindow; // Hann window
        std::vector<float> dWindowFunction; 
        
        float dCurrentAmp;                  // Amplitude tracker for RMS cutoff
        float dAmpThreshold;                // Threshold for RMS cutoff

        /**
         * YIN
         */
        double dThresh;                     // Threshold for YIN
        float convertLagToPitch(int);       // Helper function for YIN
        float dReferencePitch;              // Pitch that notes are tuned to
        std::string cNoteNames[12] = {
                "A", "A#", "B", "C", "C#", "D", 
                "D#", "E", "F", "F#", "G", "G#"
            };
        
        /**
         * pYIN
         */
        std::vector<std::pair<int, float>> findPitchCandidates();
        std::vector<float> calculateProbabilities(std::vector<std::pair<int, float>>&);
        float temporalTracking(std::vector<std::pair<int, float>>&, std::vector<float>&);

        std::vector<std::vector<float>> dPitchCandidates; // Likely pitch per frame for pYIN
        std::vector<float> dPitchProbabilities;           // Probabilities per candidate
        std::vector<float> dSmoothedPitchTrack;           // Temporal smoothing
        float dVoiceThreshold;                            // Min threshold for a freq to be considered voiced
        int dMaxCandidates;                               // Number of candidates to consider
        
 };