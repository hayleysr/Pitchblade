//Written by Austin Hills

#include "ui/RealTimeGraphVisualizer.h"

//Initializing by setting the parameters as defined
RealTimeGraphVisualizer::RealTimeGraphVisualizer(const juce::String& label, juce::Range<float> range, int updateIntervalHz){
    yAxisLabel = label;
    yAxisRange = range;
    startTimerHz(updateIntervalHz);
}

//Ensuring that the timer can't do anything if the visualizer is destroyed
RealTimeGraphVisualizer::~RealTimeGraphVisualizer(){
    stopTimer();
}

//big function to handle all visual stuff
void RealTimeGraphVisualizer::paint(juce::Graphics& g){
    //Fill background
    g.fillAll(Colors::panel);
    
    //Draw labels
    drawLabels(g);

    //Draw graph. Lock the dataMutex to ensure the audio thread doesn't modify the queue while it is being read
    //Curly brackets are used to create a new, inner scope, where objects can be created inside and not exist outside of it
    {
        juce::const_ScopedLock lock(dataMutex);
        drawGraph(g);
    }

    //Draw the threshold line
    drawThresholdLine(g);
}

void RealTimeGraphVisualizer::resized(){
    auto bounds = getLocalBounds();

    labelBounds = bounds.removeFromLeft(labelWidth);

    graphBounds = bounds;

    maxDataPoints = graphBounds.getWidth();

    //Though the window cannot currently be resized, if it ever is able to be, then excess data points must be able to be pruned
    juce::const_ScopedLock lock(dataMutex);
    while(dataQueue.size() > maxDataPoints){
        dataQueue.pop_front();
    }

}

void RealTimeGraphVisualizer::timerCallback(){
    //Only repaint if the visualizer is actually visible, otherwise don't since it'll introduce unneeded processing time
    if(isShowing()){
        repaint();
    }
}

void RealTimeGraphVisualizer::pushData(float newDataPoint){
    
}