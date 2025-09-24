// reyna macabebe

#include "Pitchblade/ui/TopBar.h"
#include <JuceHeader.h>
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "BinaryData.h"

TopBar::TopBar()
{
    //makes ui  visible
    //addAndMakeVisible(pluginTitle);

    logo.setImage(juce::ImageFileFormat::loadFrom(
        BinaryData::pitchblade_logo_png,
        BinaryData::pitchblade_logo_pngSize
    ));

    logo.setInterceptsMouseClicks(false, false);

    addAndMakeVisible(logo);
    addAndMakeVisible(settingsButton);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(presetButton);
    
}
void TopBar::paint(juce::Graphics& g)
{
    if (auto* lf = dynamic_cast<CustomLookAndFeel*>(&getLookAndFeel()))
        lf->drawPanelBackground(g, *this);
}

void TopBar::resized()
{
    // laying out each component
    auto area = getLocalBounds();

    //pluginTitle.setBounds(area.removeFromLeft(150));
    //pluginTitle.setBounds(area.removeFromLeft(150));
    auto logoArea = area.removeFromLeft(150);
    logo.setBounds(logoArea);


    
    settingsButton.setBounds(area.removeFromRight(80));
    bypassButton.setBounds(area.removeFromRight(80));
    presetButton.setBounds(area.removeFromRight(80));
    
}