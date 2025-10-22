#include "Pitchblade/effects/PitchCorrector.h"

void PitchCorrector::prepare(double sampleRate, int blockSize){
    pitchDetector.prepare(sampleRate, blockSize, 4);
    pitchShifter.prepare(sampleRate, blockSize);
    currentRatio = 1.0f;    //high correction
}
void PitchCorrector::processBlock(juce::AudioBuffer<float>& buffer){
    // Process pitch detection
    juce::AudioBuffer<float> monoBuffer(1, buffer.getNumSamples());
    monoBuffer.copyFrom(0, 0, buffer, 0, 0 ,buffer.getNumSamples());
    pitchDetector.processBlock(monoBuffer);

    float detectedPitch = pitchDetector.getCurrentPitch();
    if(detectedPitch <= 0.0f){
        pitchShifter.setPitchShiftRatio(1.0f); //bypass
        pitchShifter.processBlock(buffer);
        return;
    }

    float detectedNote = pitchDetector.getCurrentMidiNote();
    int quantizedNote = quantizeToScale(std::round(detectedNote)); //determine target note
    float targetPitch = noteToFrequency(quantizedNote);

    // Update ratio with smoothing
    float targetRatio = targetPitch / detectedPitch;
    targetRatio = juce::jlimit(0.5f, 2.0f, targetRatio);
    currentRatio = currentRatio * (1.0f - smoothing) + targetRatio * smoothing;

    pitchShifter.setPitchShiftRatio(currentRatio);
    pitchShifter.processBlock(buffer);
}
void PitchCorrector::setScale(int scaleType){
    this->scaleType = scaleType;
}
void PitchCorrector::setSmoothing(float smoothingAmt){
    smoothing = juce::jlimit(0.001f, 1.0f, smoothingAmt);
}

int PitchCorrector::quantizeToScale(int note){
    if (scale.empty()) return note;

    int closestNote = note;
    int minDist = INT_MAX;

    for (int octave = 0; octave < 8; ++octave) {
        for (int scaleNote : scale[scaleType]) {
            int candidate = scaleNote + octave * 12;
            int dist = std::abs(candidate - note);
            if (dist < minDist) {
                minDist = dist;
                closestNote = candidate;
            }
        }
    }

    return closestNote;
}
float PitchCorrector::noteToFrequency(int midi){
    return 440.0f * std::pow(2.0f, (midi - 69) / 12.0f);
}