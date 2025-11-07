#include "Pitchblade/effects/PitchCorrector.h"

void PitchCorrector::prepare(double sampleRate, int blockSize){
    pitchDetector.prepare(sampleRate, blockSize, 4);
    pitchShifter.prepare(sampleRate, blockSize);
    currentRatio = 1.0f;    //high correction
    currentMidi = 0.0f;
    prevMidi = 69.f;        //A4
    lastStableMidi = prevMidi;
    waverPhase = 0.f;   
    monoBuffer.setSize(1, blockSize);
}
void PitchCorrector::processBlock(juce::AudioBuffer<float>& buffer){
    // Process pitch detection
    prevMidi = currentMidi;
    auto numSamples = buffer.getNumSamples();
    monoBuffer.copyFrom(0, 0, buffer, 0, 0 ,buffer.getNumSamples());
    pitchDetector.processBlock(monoBuffer);

    float detectedPitch = pitchDetector.getCurrentPitch();
    if(detectedPitch <= 0.0f){
        pitchShifter.setPitchShiftRatio(1.0f); //bypass
        pitchShifter.processBlock(buffer);
        return;
    }

    currentMidi = frequencyToNote(detectedPitch);

    float detectedNote = pitchDetector.getCurrentMidiNote();
    targetMidi = (float)quantizeToScale((int)std::round(detectedNote)); //determine target note
    semitoneErrorMidi = targetMidi - currentMidi;

    correctedMidi = applyParameters(targetMidi);
    correctedPitch = noteToFrequency(correctedMidi);

    // Update ratio with smoothing
    float targetRatio = correctedPitch / detectedPitch;
    targetRatio = juce::jlimit(0.5f, 2.0f, targetRatio);

    // Smoothing
    currentRatio = currentRatio * (1.0f - smoothing) + targetRatio * smoothing;

    pitchShifter.setPitchShiftRatio(currentRatio);
    pitchShifter.processBlock(buffer);
}
float PitchCorrector::applyParameters(float &midi){

    // Note Transition: only update if change is above a particular threshold
    if(std::abs(currentMidi - lastStableMidi) > noteTransition / 100.f)
        lastStableMidi = midi;
    float activeTargetMidi = prevMidi;
    
    // Correction: apply ratio
    float correctedMidi = currentMidi + currentRatio * (targetMidi - currentMidi);

    // Retune speed: apply smoothing
    correctedMidi = prevMidi + retuneSpeed * (correctedMidi - prevMidi);
    prevMidi = correctedMidi;

    // Waver
    if(waver > 0.0f){
        float waverSemitone = (waver / 100.0f);

        waverPhase += (2.0f * juce::MathConstants<float>::pi * 6.f) / sampleRate; // Healthy vibrato is 5-6.5 Hz
        if (waverPhase > juce::MathConstants<float>::twoPi) waverPhase -= juce::MathConstants<float>::twoPi;
        correctedMidi += std::sin(waverPhase) * waverSemitone;
    }

    return correctedMidi;
}

void PitchCorrector::setScale(int scaleType){
    this->scaleType = scaleType;
}
void PitchCorrector::setSmoothing(float smoothingAmt){
    smoothing = juce::jlimit(0.001f, 1.0f, smoothingAmt);
}

int PitchCorrector::quantizeToScale(int note){
    if (scale.empty()) return note;

    int detectedOctave = note / 12;
    int closestNote = note;
    int minDist = INT_MAX;

    for (int octave = detectedOctave - 1; octave <= detectedOctave + 1; ++octave) {
        if(octave < 0 || octave > 8) continue;
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
    return 440.0f * std::pow(2.0f, (midi - 69.f) / 12.0f);
}
float PitchCorrector::frequencyToNote(int freq){
    float midi = 69.0f + 12.0f * std::log2(freq / 440.0f);
    return std::round(midi);
}

float PitchCorrector::getSemitoneError(){
    float currentPitch = pitchDetector.getCurrentPitch();
    if(currentPitch <= 0.f || targetPitch <= 0.f) return 0.f;

    float cents = 1200.0f * std::log2(currentPitch / targetPitch); 

    //clamping
    if(cents > 200.f) cents = 200.f;
    if(cents < -200.f) cents = -200.f;
    
    return cents;
}

std::string PitchCorrector::getCurrentNoteName(){
    float currentPitch = pitchDetector.getCurrentPitch();
    int pitch = frequencyToNote(currentPitch);
    int index = pitch % 12;
    if (index < 0) index += 12;
    return aNoteNames[index];
}

std::string PitchCorrector::getTargetNoteName(){
    int pitch = frequencyToNote(targetPitch);
    int index = pitch % 12;
    if (index < 0) index += 12;
    return aNoteNames[index];
}