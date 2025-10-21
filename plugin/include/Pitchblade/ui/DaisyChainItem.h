//reyna 
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/ui/ColorPalette.h"
#include "Pitchblade/ui/CustomLookAndFeel.h"

#include "Pitchblade/panels/EffectNode.h"

//merges effect buttons and bypass buttons into one row item for daisychain drag n drop
class DaisyChainItem : public juce::Component,
						public juce::DragAndDropTarget
{
public:
	DaisyChainItem(const juce::String& effectName, int index)
		:  myIndex(index)
	{
        setName(effectName);

        // drag handle (left)
        grip.setText({}, juce::dontSendNotification);
        grip.setJustificationType(juce::Justification::centred);
        grip.setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        grip.addMouseListener(this, true);                 // only grip can drag
        addAndMakeVisible(grip);

        //main effect button
		button.setButtonText(effectName);
		addAndMakeVisible(button);

        // dropdown for chaining mode
        modeButton.setButtonText("M");                       
        modeButton.setWantsKeyboardFocus(false);
        modeButton.onClick = [this]
            {
                juce::PopupMenu m;
                const bool isRoot = (myIndex == 0); // top of chain
                m.addItem(1, "Down |");
                m.addItem(2, "Split [ Double");
				// only non-root can do these
                if (!isRoot) {
                    m.addItem(3, "Double = Down");
                    m.addItem(4, "Unite > Single");
                }

                m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&modeButton),
                    [this](int result)
                    {
                        if (result > 0)
                        {
                            chainModeId = result;
                            // turn button pink when changed from default
                            auto newColor = (chainModeId == 1) ? juce::Colours::grey : Colors::accent;
                            modeButton.setColour(juce::TextButton::buttonColourId, newColor);
                            if (onModeChanged) {
                                onModeChanged(myIndex, chainModeId);
                            }
                        }
                    });
            };
        addAndMakeVisible(modeButton);

		//bypass button (right)
        bypass.setButtonText("B");
        bypass.setClickingTogglesState(false);
       
        bypass.onClick = [this]
            {
                bypassed = !bypassed;
                if (bypassed)
                    bypass.setButtonText("B"); // strikethrough B not workn rn
                else
                    bypass.setButtonText("B");

                if (onBypassChanged)
                    onBypassChanged(myIndex, bypassed);

                if (bypassed) {
                    bypass.setColour(juce::TextButton::buttonColourId, Colors::accent);
                } else {
                    bypass.setColour(juce::TextButton::buttonColourId, Colors::panel);
                }
            };

        addAndMakeVisible(bypass);
        // Set initial size
		setSize(200, 40);               
        setInterceptsMouseClicks(true, true);  
	} 

    //layout
	void resized() override
	{
        auto area = getLocalBounds().reduced(5);

        // grip : left drag handle
        auto gripArea = area.removeFromLeft(20);
        grip.setBounds(gripArea.withTrimmedTop(8).withHeight(20));

        // bypass toggle : next to dropdown, smaller
        auto bypassArea = area.removeFromRight(28);

        bypass.setBounds(bypassArea.reduced(0));
        // dropdown : right side, inside effect btn
        button.setBounds(area);
        auto chevronArea = area.removeFromRight(30);
        modeButton.setBounds(chevronArea.reduced(0));
        modeButton.toFront(false); 

        button.setBounds(area);
	}

	//for external bypass changes, changes button color if gobal bypassed
    void updateBypassVisual(bool state)
    {
        bypassed = state;

        const auto bg = state ? juce::Colours::hotpink : Colors::panel;
        bypass.setColour(juce::TextButton::buttonColourId, bg);
        bypass.setColour(juce::TextButton::buttonOnColourId, bg);
        bypass.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        bypass.setColour(juce::TextButton::textColourOnId, juce::Colours::white);

        bypass.repaint();
    }

	// drag and drop
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isLeftButtonDown() && (e.eventComponent == &grip || e.originalComponent == &grip)) 
{
            if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this)) {
                // create a snapshot image of component for dragging
                auto snapshot = createComponentSnapshot(getLocalBounds());
                container->startDragging(getName(), this, snapshot, true); 
            }
        }
    }
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details) override {
        juce::ignoreUnused(details);
        return true;
    }

    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& details) override {
        juce::ignoreUnused(details);
        // default: highlight whole row
        //setAlpha(0.7f);
    }

    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& details) override {
        setAlpha(1.0f);
        showDropAbove = false;
        repaint();
    }

    void itemDragMove(const juce::DragAndDropTarget::SourceDetails& details) override {
        auto local = details.localPosition;
		// halfway point 
        showDropAbove = (local.getY() < getHeight() / 2);
        repaint();
    }

    void itemDropped(const juce::DragAndDropTarget::SourceDetails& details) override
    {
        if (onReorder)
        {
            int targetIndex = myIndex;
            if (!showDropAbove) targetIndex++;   // bottom half = insert after
            onReorder(-1, details.description.toString(), targetIndex);
        }
        showDropAbove = false;
        repaint();
    }

    //line showing where u dragging///////////////////////////
    void paint(juce::Graphics& g) override
        {
            if (showDropAbove)
            {
                g.setColour(juce::Colours::pink); // pink
                g.fillRect(0, 0, getWidth(), 3);  // line at top
            }

            // draw grip dots
            g.setColour(juce::Colours::grey);
            auto r = grip.getBounds().toFloat();
            for (int y = 0; y < 3; ++y)
            g.fillEllipse(r.getX() + r.getWidth() / 2 - 2, r.getY() + 4 + y * 8, 4, 4);
        }

    /////////////////////////////////////////////////////////////////////////////

    std::function<void(int, juce::String, int)> onReorder;

	juce::TextButton button;    // main effect button
	//juce::ToggleButton bypass;  // bypass toggle
    juce::TextButton bypass;
    bool bypassed = false;
	juce::Label grip;           // drag handle
    juce::TextButton modeButton;   // chain mode 
    int chainModeId = 1;

    int getIndex() const { return myIndex; }
    std::function<void(int, bool)> onBypassChanged;     //row index, bypass
	std::function<void(int, int)> onModeChanged;        //row index, mode id
    bool showDropAbove = false;

private:
    int myIndex;
};

