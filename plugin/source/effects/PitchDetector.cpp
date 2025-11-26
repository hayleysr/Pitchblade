/**
 * Author: Hayley Spellicy-Ryan
 * 
 * Implements the pYIN algorithm to detect pitch.
 * Pitch is determined by the frequency of the waveform, which is the inverse of the period.
 * Detecting pitch relies on detecting the period.
 * 
 * The YIN algorithm overlaps a waveform with itself over a lag parameter 
 * and calculates a difference function to determine the most likely period.
 * 
 * The pYIN algorithm takes an array of likely periods and applies smoothing over time.
 * 
 */

 #include "Pitchblade/effects/PitchDetector.h"

 PitchDetector::PitchDetector(int windowSize, float referencePitch):
    dWindowSize(windowSize),
    dYinBufferSize(windowSize / 2),
    dReferencePitch(referencePitch),
    dVoiceThreshold(0.1f),
    dMaxCandidates(4),
    dAmpThreshold(0.001f)
 {
    // Constructor
 }

 // Defaults reference pitch to 440Hz, standard A
 PitchDetector::PitchDetector(int windowSize) : PitchDetector(windowSize, 440) {

 }

 // Defaults Hann window size to 1024. Higher values increase resolution, lower values increase speed.
 PitchDetector::PitchDetector() : PitchDetector(1024, 440) {

 }

 PitchDetector::~PitchDetector()
 {
    // Destructor
 }
 
 void PitchDetector::prepare(double sampleRate, int samplesPerBlock, double hopSizeDenominator = 4)
 {
    this->sampleRate = sampleRate;
    frame.assign(dWindowSize, 0.0f);
    r.assign(dYinBufferSize + 1, 0.0f);

    // Initialize circular buffer of size windowSize with empty floats
    for(int i = 0; i < dWindowSize; ++i)
        dCircularBuffer.push_back(0.0f);
    dCircularIdx = 0;

    // Set hop size to fraction of window size. Set higher for more resolution, lower for better CPU
    dHopSize = dWindowSize / hopSizeDenominator;
    dSamplesUntilHop = dHopSize; // Initialize counter

    // Initialize yin buffer
    for(int i = 0; i < dYinBufferSize; ++i)
        dYinBuffer.push_back(0.0f);

    // Define Hann window
    for(int i = 0; i < dWindowSize; ++i)
        dWindowFunction.push_back(0.0f);
        
    for (int i = 0; i < dWindowSize; ++i) {
        dWindowFunction[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (dWindowSize - 1)));
    }

    pitchCandidates.clear();
    pitchProbabilities.clear();
    smoothedPitchTrack.clear();
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

        dSamplesUntilHop--; // Decrement hop counter

        if(dSamplesUntilHop <= 0){
            // Handle wrap-around and apply windowing function
            int startIndex = (dCircularIdx - dWindowSize + dWindowSize) % dWindowSize;
            for (int i = 0; i < dWindowSize; ++i) {
                int index = (startIndex + i) % dWindowSize;
                frame[i] = dCircularBuffer[index] * dWindowFunction[i];
            }

            // Pass to processor to calculate pYIN
            processFrame(frame);

            dSamplesUntilHop += dHopSize; // Reset counter
        }
    }

 }

 void PitchDetector::processFrame(const std::vector<float>& frame)
 {
    dCurrentAmp = calculateRMS(frame);  // Check if amp is below threshold
    if(dCurrentAmp < dAmpThreshold){
        currentPitch = 0.0f;           // Set pitch to 0
        previousCandidates.clear();    // Reset Viterbi
        return;
    }

    difference(frame);      // Populate dYinBuffer with difference function
    cumulative();           // Apply cumulative mean to dYinBuffer

    auto candidates = findPitchCandidates();
    //std::vector<float> probabilities = calculateProbabilities(candidates);
    //currentPitch = temporalTracking(candidates, probabilities);

    currentPitch = processViterbi(candidates);
 }

 void PitchDetector::difference(const std::vector<float>& frame)
 {
    // ACF at lag 0
    float sumSquares = 0.0f;
    for(int i = 0; i < dWindowSize; ++i){
        sumSquares += frame[i] * frame[i];  //from ACF = sum_{j=t+1}^{t+W}(x_j*x_{j+\tau})
    }

    // Running sum
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
                - (frame[tau - 1] * frame[tau - 1]);

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

 // Estimate optimization curve using parabolic interpretation for sample accuracy
 float PitchDetector::parabolicMinimum(int tau)
 {
    if(tau <= 0 || tau >= dYinBufferSize+1) return (float)tau;

    float x = (float) tau;  //for x = tau find nearest y to left and right
    float y1 = dYinBuffer[tau - 1];
    float y2 = dYinBuffer[tau];
    float y3 = dYinBuffer[tau + 1];

    float denominator = 2 * (2* y2-y1-y3);
    if(std::abs(denominator) < 0.0001) return x;

    float delta = (y3 - y1) / denominator;
    return x + delta;
 }

 std::vector<std::pair<int, float>> PitchDetector::findPitchCandidates()
 {
    std::vector<std::pair<int, float>> pitchCandidates;
    
    // Find all local minima below threshold
    for(int tau = 2; tau < dYinBufferSize-1; ++tau){
        // If it is a local minimum, it is a candidate
        if(dYinBuffer[tau] < dYinBuffer[tau-1] && dYinBuffer[tau] < dYinBuffer[tau+1]){
            pitchCandidates.emplace_back(tau, dYinBuffer[tau]);
        }
    }

    // Fallback for silence: find global min
    if(pitchCandidates.empty()){
        float minVal = std::numeric_limits<float>::max();
        int minTau = -1;
        for (int tau = 2; tau < dYinBufferSize - 1; ++tau) {
            if (dYinBuffer[tau] < minVal) {
                minVal = dYinBuffer[tau];
                minTau = tau;
            }
        }
        if (minTau > 0) pitchCandidates.emplace_back(minTau, minVal);
    }

    return pitchCandidates;
 }

 // DP algorithm for most probable hidden state sequence
 // in this case, finding the most likely pitch given the pitch context
 float PitchDetector::processViterbi(std::vector<std::pair<int, float>>& rawCandidates)
 {
    std::vector<PitchCandidate> currentCandidates;  

    //1. use parabolic interpolation to get pitch-probability-cost objects for each candidate
    for(auto&p : rawCandidates){
        PitchCandidate c;

        //parabolic interpolation
        float parabolicLag = parabolicMinimum(p.first);
        c.pitch = convertLagToPitch(parabolicLag);

        if(p.second < 0.001f) p.second = 0.001f;
        c.probability = 1.0f - p.second;
        if(c.probability < 0) c.probability = 0;

        c.cost = 0.0f;
        currentCandidates.push_back(c);
    }

    //safeguard against white noise
    if(currentCandidates.empty()){
        previousCandidates.clear();
        return 0.f; 
    }

    //2. Initialize if in initial state
    if(previousCandidates.empty()){
        previousCandidates = currentCandidates;
        auto best = std::max_element(currentCandidates.begin(), currentCandidates.end(),
            [](const PitchCandidate& a, const PitchCandidate& b){return a.probability < b.probability; });
        return best->pitch;
    }

    //3. Induction: cheapest path from prev to current
    // Cost function: (1 - prob) + transitionCost * log2(pitchdiff)
    int bestCandidate_x = 0;    //index of best candidate
    float minGlobalCost = std::numeric_limits<float>::max();    //spawn minimum cost at max

    for(int i = 0; i < currentCandidates.size(); ++i){
        float minPathCost = std::numeric_limits<float>::max(); // minimum for this particular path

        // Go through all previous notes to find the shortest path from current note to previous
        for(const auto&prev : previousCandidates){
            // Calculate distance between pitches
            float pitchRatio = currentCandidates[i].pitch / (prev.pitch + 0.001f);
            float dist = std::abs(std::log2(pitchRatio));

            // Penalize large distance, to cut transients
            float penalty = transitionCost * dist;

            // Cost to get to prev + jump penalty + unlikelihood of note (from step 1)
            float currentCost = prev.cost + penalty + (1.f - currentCandidates[i].probability);

            // Update minimum
            if(currentCost < minPathCost) minPathCost = currentCost;
        }

        // Handle last index
        currentCandidates[i].cost = minPathCost;

        // Update new minimum
        if(minPathCost < minGlobalCost){
            minGlobalCost = minPathCost;
            bestCandidate_x = i;
        }
    }

    //4. Update state
    previousCandidates = currentCandidates;

    //5. Return ideal pitch + gate to cut the transients
    if(currentCandidates[bestCandidate_x].probability < dVoiceThreshold) return 0.f;
    return currentCandidates[bestCandidate_x].pitch;
 }

 std::vector<float> PitchDetector::calculateProbabilities(std::vector<std::pair<int, float>>& candidates)
 {
    std::vector<float> probabilities;
    if(candidates.empty()) return probabilities;

    // YIN to probabilities
    float sum = 0.0f;
    for (const auto& candidate : candidates) {
        float prob = std::exp(-candidate.second * 10.0f);
        probabilities.push_back(prob);
        sum += prob;
    }
    
    // Normalize
    if (sum > 0.0f) {
        for (float& prob : probabilities) {
            prob /= sum;
        }
    }
    
    return probabilities;
 }

 // Viterbi algorithm
 float PitchDetector::temporalTracking(std::vector<std::pair<int, float>>& candidates, std::vector<float>& probabilities)
 {
    if(candidates.empty()) return 0.0f;
    float weightedSum = 0.0f;
    float totalWeight = 0.0f;

    for (int i = 0; i < candidates.size(); ++i) {
        float pitch = convertLagToPitch(candidates[i].first);
        weightedSum += pitch * probabilities[i];
        totalWeight += probabilities[i];
    }
    
    // Detect if voiced: if the signal is periodic or noisy
    float bestProbability = probabilities.empty() ? 0.0f : probabilities[0];
    if (bestProbability < dVoiceThreshold) {
        return 0.0f; 
    }
    
    return weightedSum / totalWeight;
 }

 float PitchDetector::calculateRMS(const std::vector<float>& frame)
 {
    float sumSquares = 0.0f;
    for (float sample : frame) {
        sumSquares += sample * sample;
    }
    return std::sqrt(sumSquares / frame.size());
 }

 float PitchDetector::convertLagToPitch(float lag)
 {
    if (lag <= 0) return 0.0f;
    return static_cast<float>(sampleRate) / static_cast<float>(lag);
 }

 float PitchDetector::getCurrentPitch()
 {
    return currentPitch;
 }

 float PitchDetector::getCurrentMidiNote()
{
    if (currentPitch <= 0.f) return 0.f;   
    return (int)(round(69.0f + 12.0f * log2(currentPitch / 440.0f)));
}

 /**
  * Returns number of semitones above or below reference pitch
  * f = f₀ * 2^(n/12)
  * n = 12log_2(f / f₀)
  */
 float PitchDetector::getCurrentNote()
 {
    if (currentPitch <= 0.f) return 0.f;
    return 12 * std::log2(currentPitch / dReferencePitch);
 }

 float PitchDetector::getSemitoneError()
 {
    return getCurrentNote() - getCurrentPitch();
 }

 std::string PitchDetector::getCurrentNoteName()
 {
    int index = (int)(getCurrentNote()) % 12;
    if (index < 0) index += 12;
    return cNoteNames[index];
 }