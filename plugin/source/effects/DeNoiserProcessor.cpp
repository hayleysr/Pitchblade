//Austin Hills

#include "Pitchblade/effects/DeNoiserProcessor.h"

//Constructor
DeNoiserProcessor::DeNoiserProcessor() :
    forwardFFT(fftOrder),
    window(fftSize,juce::dsp::WindowingFunction<float>::hann)
{
    //Initialize buffers
    inputBuffer.resize(fftSize,0.0f);
    outputBuffer.resize(fftSize,0.0f);
    fftData.resize(fftSize * 2,0.0f);
    noiseProfile.resize(fftSize / 2 + 1,0.0f);
}

void DeNoiserProcessor::prepare(const double sRate){
    sampleRate = sRate;

    //reset buffers and state
    std::fill(inputBuffer.begin(),inputBuffer.end(),0.0f);
    std::fill(outputBuffer.begin(),outputBuffer.end(),0.0f);
    std::fill(noiseProfile.begin(),noiseProfile.end(),0.0f);
    std::fill(fftData.begin(),fftData.end(),0.0f);

    inputBufferPos = 0;
    outputBufferPos = 0;
    samplesProcessed = 0;
    noiseProfileSamples = 0;
}

//Setters for user controlled parameters
void DeNoiserProcessor::setReduction(float reduction){
    reductionAmount = reduction;
}

void DeNoiserProcessor::setLearning(bool learning){
    if(learning){
        //If starting to learn, reset the noise profile
        if(!isLearning){
            std::fill(noiseProfile.begin(),noiseProfile.end(),0.0f);
            noiseProfileSamples = 0;
        }
        isLearning = true;
    }else{
        //If stopping learning, average the collected profile
        if(isLearning && (noiseProfileSamples > 0)){
            for(int i = 0; i < noiseProfile.size(); i++){
                float& bin = noiseProfile[i];
                bin /= (float)noiseProfileSamples;
            }
        }

        isLearning = false;
    }
}

//Main processing loop
void DeNoiserProcessor::process(juce::AudioBuffer<float>& buffer){
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for(int i = 0; i < numSamples; i++){
        //Get mono mix sample
        float inputSample = 0.0f;
        for(int j = 0; j < numChannels; j++){
            inputSample += buffer.getSample(j,i);
        }
        if(numChannels > 0){
            inputSample /= (float)numChannels;
        }

        //Overlap add input
        inputBuffer[inputBufferPos++] = inputSample;

        //Get output sample
        float outputSample = outputBuffer[outputBufferPos++];

        //Write output sample to all channels
        for(int j = 0; j < numChannels; j++){
            buffer.setSample(j,i,outputSample);
        }

        //Process frame
        //If the input buffer's position is equal to the size of the fft, then send it off for processing
        if(inputBufferPos == fftSize){
            processFrame();

            //Shift input buffer to prepare for the next samples
            std::memmove(inputBuffer.data(),inputBuffer.data() + hopSize,overlap * sizeof(float));
            //Clear the end of the buffer
            std::fill(inputBuffer.data() + overlap,inputBuffer.data() + fftSize, 0.0f);
            //Reset the write pointer
            inputBufferPos = overlap;

            //Moving output buffer logic up to see if it fixes the choppy audio
            std::memmove(outputBuffer.data(),outputBuffer.data() + hopSize,overlap * sizeof(float));
            std::fill(outputBuffer.data() + overlap,outputBuffer.data() + fftSize,0.0f);
            outputBufferPos = 0;
        }

        //Moved this stuff up because there was choppy audio
        // //If the output buffer's position is equal to the size of the fft, then do the same as above but for the output, preparing the cleaned audio to be sent out
        // if(outputBufferPos == fftSize){
        //     //Shift output buffer
        //     std::memmove(outputBuffer.data(),outputBuffer.data() + hopSize,overlap * sizeof(float));
        //     std::fill(outputBuffer.data() + overlap,outputBuffer.data() + fftSize,0.0f);
        //     outputBufferPos = 0;
        // }
    }
}

//Processing of the frame, either for learning or cleaning data
void DeNoiserProcessor::processFrame(){
    //Window the input buffer
    std::copy(inputBuffer.begin(),inputBuffer.end(),fftData.begin());
    window.multiplyWithWindowingTable(fftData.data(),fftSize);

    //Clear imaginary part
    std::fill(fftData.data() + fftSize,fftData.data() + fftSize * 2, 0.0f);

    //Perform forward FFT
    forwardFFT.performRealOnlyForwardTransform(fftData.data());

    //Process magnitudes and phases so fftData contains complex numbers in packed format
    const int numBins = fftSize / 2 + 1;
    
    //Storing the value of isLearning within this context so that the next part happens faster
    bool learning = isLearning;

    //Cycling through
    for(int i = 0; i < numBins; i++){
        float real;
        float imag;
        //Unpacking parts for the ith frequency bins
        if(i==0 || (i == (numBins - 1))){
            real = fftData[i];
            imag = 0.0f;
        }else{
            real = fftData[i * 2];
            imag = fftData[i * 2 + 1];
        }

        //Magnitude is the strength of the frequency
        float magnitude = std::sqrt(real * real + imag * imag);
        //Phase is the timing if the frequency's wave (from what I understand). Messing with this will do bad things to the audio, so leave it unchanged
        float phase = std::atan2(imag, real);

        if(learning){
            //Accumulate info for noise profile
            noiseProfile[i] += magnitude;
        }else{
            //Spectral subtraction
            //This part is the actual denoising part that uses the noiseProfile and reductionAmount as keys to determine how much and what to reduce
            float noiseMag = noiseProfile[i];
            //Calculates the amount to subtract from a given frequency based on the average magnitude during learning.
            //Doubling allows the sweet spot to be close to 50% of the slider with the ability to subtract more if desired 
            float reduction = noiseMag * (reductionAmount * 2.0f);

            //Make reduce the magnitude of the frequency using the reduction. If that results in a value below 0, just use 0 instead
            float reducedMag = std::max(0.0f, magnitude - reduction);

            //If the sound is complete silence, then don't completely make it empty. Give a little bit of noise (can be removed with noise gate)
            //This is to prevent artifacting in the sound 
            float floor = magnitude * (1.0f - reductionAmount);

            //This is the final decision, and it chooses the greater value between the reducedMag and the floor value
            magnitude = std::max(reducedMag,floor);
        }

        //Reconstruct the frequencies
        //If statement handles the special first and last bins, which have 0 for imag
        if(i == 0 || i == numBins - 1){
            fftData[i] = magnitude;
        }else{
            //Calculate new real part
            fftData[i * 2] = magnitude * std::cos(phase);
            //Calculate new imag part
            fftData[i * 2 + 1] = magnitude * std::sin(phase);
        }
    }

    //increment noiseProfileSamples if it is learning for use in averaging later on
    if(learning){
        noiseProfileSamples++;
    }

    //Performing the inverse FFT function! This is putting those pieces back together into an actual bit of audio!
    forwardFFT.performRealOnlyInverseTransform(fftData.data());

    //Apply the window to the processed audio sitting in fftData
    window.multiplyWithWindowingTable(fftData.data(),fftSize);

    //Normalize the audio so its level is correct
    juce::FloatVectorOperations::multiply(fftData.data(),1.0f/1.5f,fftSize);

    //Add the processed sample back to the output buffer
    for(int i = 0; i < fftSize; i++){
        outputBuffer[i] += fftData[i];
    }
}