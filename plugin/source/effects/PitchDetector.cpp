/**
 * Author: Hayley Spellicy-Ryan
 */

 #include "Pitchblade/effects/PitchDetector.h"

 PitchDetector::PitchDetector(int windowSize, float referencePitch):
    dWindowSize(windowSize),
    dYinBufferSize(windowSize / 2),
    dReferencePitch(referencePitch)
 {
    // Constructor
 }

 PitchDetector::PitchDetector(int windowSize) : PitchDetector(windowSize, 440) {

 }

 PitchDetector::PitchDetector() : PitchDetector(1024, 440) {

 }

 PitchDetector::~PitchDetector()
 {
    // Destructor
 }
 
 void PitchDetector::prepare(double sampleRate, int samplesPerBlock, double hopSizeDenominator = 4)
 {
    dSampleRate = sampleRate;

    // Initialize circular buffer of size windowSize with empty floats
    for(int i = 0; i < dWindowSize; ++i)
        dCircularBuffer.push_back(0.0f);
    dCircularIdx = 0;

    // Set hop size to fraction of window size. Set higher for more resolution, lower for better CPU
    dHopSize = dWindowSize / hopSizeDenominator;

    // Initialize yin buffer
    for(int i = 0; i < dYinBufferSize; ++i)
        dYinBuffer.push_back(0.0f);

    // Define Hann window
    for(int i = 0; i < dWindowSize; ++i)
        dWindowFunction.push_back(0.0f);
        
    for (int i = 0; i < dWindowSize; ++i) {
        dWindowFunction[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (dWindowSize - 1)));
    }
 }

 void PitchDetector::processBlock(const juce::AudioBuffer<float> &buffer)
 {
    // Set info pointers
    auto* bufferData = buffer.getReadPointer(0);
    auto bufferNumSamples = buffer.getNumSamples();  

    // Accumulate incoming samples in circular buffer
    for(int i = 0; i < bufferNumSamples; ++i){
        dCircularBuffer[dCircularIdx] = bufferData[i];
        dCircularIdx = (dCircularIdx + 1) % dWindowSize;
    }

    // Copy accumulated frames to handle wrap-around
    std::vector<float> frame(dWindowSize, 0.0f);
    int startIndex = (dCircularIdx - dWindowSize + dWindowSize) % dWindowSize;
    for (int i = 0; i < dWindowSize; ++i) {
        int index = (startIndex + i) % dWindowSize;
        frame[i] = dCircularBuffer[index];
    }

    // Apply windowing function
    for (int n = 0; n < dWindowSize; ++n) {
        frame[n] *= dWindowFunction[n];
    }

    // Pass to processor to calculate pYIN
    processFrame(frame);

 }

 void PitchDetector::processFrame(const std::vector<float>& frame)
 {
    difference(frame);      // Populate dYinBuffer with difference function
    cumulative();           // Apply cumulative mean to dYinBuffer
    dCurrentPitch = absoluteThreshold();    //yin algorithm
 }

 void PitchDetector::difference(const std::vector<float>& frame)
 {
    // ACF at lag 0
    float sumSquares = 0.0f;
    for(int i = 0; i < dWindowSize; ++i){
        sumSquares += frame[i] * frame[i];  //from ACF = sum_{j=t+1}^{t+W}(x_j*x_{j+\tau})
    }

    // Running sum
    std::vector<float> r(dYinBufferSize + 1, 0.0f);
    r[0] = sumSquares;

    // from DF(tau) = r_{\tau}(0) + r_{t + \tau}(0) - 2r_t(\tau)
    for(int tau = 1; tau < dYinBufferSize; ++tau){
        // Calculate ACF sum for this lag
        float acf = 0.0f;
        for(int j = 0; j < dWindowSize - tau; ++j){
            acf += frame[j] * frame[j + tau]; //from ACF = sum_{j=t+1}^{t+W}(x_j*x_{j+\tau})
        }

        // Running sum: lag for prev, but delete oldest sample and add newest
        r[tau] = r[tau - 1] 
                - (frame[tau - 1] * frame[tau - 1]) 
                + (frame[dWindowSize - tau] * frame[dWindowSize - tau]);

        // DF
        dYinBuffer[tau] = r[0] + r[tau] - 2 * acf;
    }
    dYinBuffer[0] = 1.0f; // Avoid div by 0
 }

 void PitchDetector::cumulative()
 {   
    // Running sum
    float r = 0;
    dYinBuffer[0] = 1.0f; // Special case
    
    // Cumulative mean normalization
    for (int tau = 1; tau < dYinBufferSize; ++tau) {
        r += dYinBuffer[tau];
        if (r <= 0.0f)
        {
            dYinBuffer[tau] = 1;
        } else {
            dYinBuffer[tau] *= tau / r;
        }
    }  
 }

 int PitchDetector::absoluteThreshold()
 {
    const float thresh = 0.15f;
    int tau = 2;
    int minTau = -1;
    float minVal = std::numeric_limits<float>::max(); // Initialize min to max float
    
    while (tau < dYinBufferSize)
    {
        if (dYinBuffer[tau] < thresh)
        {
            // Find smallest period tau for which d' has local minimum
            while (tau + 1 < dYinBufferSize && dYinBuffer[tau + 1] < dYinBuffer[tau])
            {
                ++tau;
            }
            return convertLagToPitch(static_cast<int>(tau));
        } 
        else 
        {
            // Fallback: global min
            if (dYinBuffer[tau] < minVal)
            {
                minVal = dYinBuffer[tau];
                minTau = tau;
            }
        }
        ++tau;
    }
    
    // No good pitch found
    if (minTau > 0)
        return convertLagToPitch(static_cast<int>(minTau));
    
    return 0.0f; // No pitch found
 }

 float PitchDetector::convertLagToPitch(int lag)
 {
    if (lag <= 0) return 0.0f;
    return static_cast<float>(dSampleRate) / static_cast<float>(lag);
 }

 float PitchDetector::getCurrentPitch()
 {
    return dCurrentPitch;
 }

 /**
  * Returns number of semitones above or below reference pitch
  * f = f₀ * 2^(n/12)
  * n = 12log_2(f / f₀)
  */
 float PitchDetector::getCurrentNote()
 {
    return 12 * std::log2(dCurrentPitch / dReferencePitch);
 }

 std::string PitchDetector::getCurrentNoteName()
 {
    int index = (int)getCurrentNote() % 12;
    if (index < 0) index += 12;
    return cNoteNames[index];
 }