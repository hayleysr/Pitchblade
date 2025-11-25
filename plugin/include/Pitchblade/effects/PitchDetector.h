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

 // Public interface class for testing
 class IPitchDetector{
 public:
    virtual ~IPitchDetector() = default;
    virtual void prepare(double, int, double) = 0;
    virtual void processBlock(const juce::AudioBuffer<float>&) = 0;
    virtual float getCurrentPitch() = 0;
    virtual float getCurrentMidiNote() = 0;
 };

 struct PitchCandidate{
    float pitch;
    float probability;
    float cost;
 };
 
 class PitchDetector : public IPitchDetector{
    public:
        // Constructor
        PitchDetector(int, float);
        PitchDetector(int);
        PitchDetector();

        // Prepare detector
        void prepare(double, int, double) override;

        // Process a block of audio
        void processBlock(const juce::AudioBuffer<float>&) override;

        void processFrame(const std::vector<float>&);

        float getCurrentPitch() override;
        float getSemitoneError();
        float getCurrentNote();
        float getCurrentMidiNote() override;
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

        float currentPitch;                // Pitch of most recent sample batch in Hz
        double sampleRate;                 // Sample rate
        int dWindowSize;                    // interval i to 2W
        int dYinBufferSize;                 // W, on sum j = t + 1 to t + W
        int dLag;                           // Lag
        std::vector<float> dYinBuffer;      // YIN buffer
        std::vector<float> dCircularBuffer; // Accumulative buffer of samples from AudioBuffer
        int dCircularIdx;                   // Start position in circular buffer
        int dHopSize;                       // Amount to jump fwd by. Creates overlapping frames.
        int dSamplesUntilHop;
        juce::dsp::WindowingFunction<float>::WindowingMethod dWindow; // Hann window
        std::vector<float> dWindowFunction; 
        std::vector<float> frame;
        std::vector<float> r;
        
        float dCurrentAmp;                  // Amplitude tracker for RMS cutoff
        float dAmpThreshold;                // Threshold for RMS cutoff

        /**
         * YIN
         */
        double dThresh;                     // Threshold for YIN
        float convertLagToPitch(float);     // Helper function for YIN
        float dReferencePitch;              // Pitch that notes are tuned to
        std::string cNoteNames[12] = {
                "A", "A#", "B", "C", "C#", "D", 
                "D#", "E", "F", "F#", "G", "G#"
            };
        
        /**
         * pYIN
         */
        std::vector<float> calculateProbabilities(std::vector<std::pair<int, float>>&);
        float temporalTracking(std::vector<std::pair<int, float>>&, std::vector<float>&);

        std::vector<std::vector<float>> pitchCandidates; // Likely pitch per frame for pYIN
        std::vector<float> pitchProbabilities;           // Probabilities per candidate
        std::vector<float> smoothedPitchTrack;           // Temporal smoothing
        float dVoiceThreshold;                            // Min threshold for a freq to be considered voiced
        int dMaxCandidates;                               // Number of candidates to consider

        /**
         * Viterbi
         */
        std::vector<PitchCandidate> previousCandidates;
        float transitionCost = 15.f; // Penalty for changing pitch
        float parabolicMinimum(int);
        std::vector<std::pair<int, float>> findPitchCandidates();
        float processViterbi(std::vector<std::pair<int, float>>&);
 };