//Written by Austin Hills

#include "Pitchblade/ui/RealTimeGraphVisualizer.h"

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
        juce::ScopedLock lock(dataMutex);
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
    juce::ScopedLock lock(dataMutex);
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

//This function is called from the audio thread to push new data in
void RealTimeGraphVisualizer::pushData(float newDataPoint){
    //Lock mutex to gain access
    juce::ScopedLock lock(dataMutex);

    //Add new data in the back of the queue
    dataQueue.push_back(newDataPoint);

    //Purge old data if the queue is now larger than graph width
    while(dataQueue.size() > maxDataPoints){
        dataQueue.pop_front();
    }

    //Mutex is unlocked automatically when it goes out of scope
}

//This function is called from the message thread when the threshold's slider value has changed
void RealTimeGraphVisualizer::setThreshold(float yValue, bool enabled){
    thresholdValue = yValue;
    thresholdEnabled = enabled;
}

//Draw labels
void RealTimeGraphVisualizer::drawLabels(juce::Graphics& g){
    g.setColour(Colors::buttonText);
    g.setFont(12.0f);

    //Draw border
    g.drawRect(graphBounds);

    //Draw y axis label (often dB)
    g.drawText(yAxisLabel,labelBounds.reduced(2),juce::Justification::centred,1);

    //Maximum value at the top right
    g.drawText(juce::String(yAxisRange.getEnd()),labelBounds.getX(),graphBounds.getY()-6,labelWidth,12,juce::Justification::centredRight);

    //Minimum value at the bottom right
    g.drawText(juce::String(yAxisRange.getStart()),labelBounds.getX(),graphBounds.getBottom()-6,labelWidth,12,juce::Justification::centredRight);

}

//Graph drawing function. The function assumes that the dataMutex is already locked
void RealTimeGraphVisualizer::drawGraph(juce::Graphics& g){
    //If there is nothing to draw, do not draw
    if(dataQueue.empty()){
        return;
    }

    juce::Path graphPath;

    g.setColour(Colors::accent);

    //Get graph dimensions

    const float graphW = (float)graphBounds.getWidth();
    const float graphH = (float)graphBounds.getHeight();
    const float graphY = (float)graphBounds.getY();
    const float graphB = (float)graphBounds.getBottom();

    //Get the first data point to start the path
    float firstValue = dataQueue.front();

    //jmap maps a value from one range to another. The data value is mapped to the coordinate range
    //Please note that the pixel range is from bottom to top
    float startY = juce::jmap(firstValue,yAxisRange.getStart(), yAxisRange.getEnd(), graphB, graphY);

    //Start the path at the first data point at the left side of the graph
    graphPath.startNewSubPath(graphBounds.getX(),startY);

    //Calculate how many pixels wide each data point is
    float stepX = graphW / (float)maxDataPoints;
    float currentX = (float)graphBounds.getX() + stepX;

    //Add remaining points to the path
    for(size_t i = 1; i < dataQueue.size(); i++){
        float value = dataQueue[i];
        float y = juce::jmap(value,yAxisRange.getStart(),yAxisRange.getEnd(),graphB,graphY);
        graphPath.lineTo(currentX,y);
        currentX+=stepX;
    }

    //Draw the path to the screen with a 2 pixel stroke
    g.strokePath(graphPath,juce::PathStrokeType(2.0f));
}

//Draw threshold line
void RealTimeGraphVisualizer::drawThresholdLine(juce::Graphics& g){
    //If there is nothing to draw, then do not draw
    if(!thresholdEnabled){
        return;
    }

    //Map the current threshold value to a y-pixel coordinate
    float lineY = juce::jmap(thresholdValue,yAxisRange.getStart(),yAxisRange.getEnd(),(float)graphBounds.getBottom(),(float)graphBounds.getY());

    //Ensure that the value does not appear outside of the graph bounds, just in case
    lineY = juce::jlimit((float)graphBounds.getY(),(float)graphBounds.getBottom(),lineY);

    g.setColour(Colors::accent);

    //Define the dash pattern
    float dashes[] = {4.0f,4.0f};

    //Draw the dashed line across the graph horizontally
    g.drawDashedLine(juce::Line<float>((float)graphBounds.getX(),lineY,(float)graphBounds.getRight(),lineY),dashes,2);
}