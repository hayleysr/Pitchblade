//reyna
// used to add tooltip text to components
#pragma once
#include <JuceHeader.h>
#include <unordered_map>

class TooltipManager {
public:
	//TooltipManager() = default;     // default constructor
    TooltipManager()
    {
        // Create tooltip window and keep it always on top
        tooltipWindow = std::make_unique<juce::TooltipWindow>(nullptr, 0);
        tooltipWindow->setAlwaysOnTop(true);
        tooltipWindow->setMillisecondsBeforeTipAppears(400); // optional delay
    }

	// load tooltips from file 
    void loadTooltipsFromFile(const juce::File& file) {
        if (!file.existsAsFile())
            return;

        juce::StringArray lines;  
		file.readLines(lines);      // read line by line
        
        for (auto& line : lines) {
			if (line.trim().isEmpty() || line.trim().startsWithChar('//')) {     // skip empty lines and comments
                continue;
            }
			auto parts = juce::StringArray::fromTokens(line, "=", "");          // split where "=" is in file
            if (parts.size() == 2) {
				tooltips[parts[0].trim()] = parts[1].trim();    // store in map
            }
        }
    }

	// get tooltip for a given part
    juce::String getTooltipFor(const juce::String& part) const {
        if (auto it = tooltips.find(part); it != tooltips.end())
            return it->second;
        return {};
    }

    //apply tooltip
    void applyTooltipTo(juce::Component& comp, const juce::String& partName) {
        if (auto text = getTooltipFor(partName); !text.isEmpty())
            comp.getProperties().set("tooltip", text); 

        // keep the floating window above overlays
        tooltipWindow->toFront(true);
    }

    juce::TooltipWindow* getWindow() { return tooltipWindow.get(); }

private:
	// map of part names to tooltips
    std::unordered_map<juce::String, juce::String> tooltips;
    std::unique_ptr<juce::TooltipWindow> tooltipWindow;
};
