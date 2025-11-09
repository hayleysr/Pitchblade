//reyna 
#pragma once
#include <JuceHeader.h>
#include "Pitchblade/ui/CustomLookAndFeel.h"
#include "Pitchblade/ui/ColorPalette.h"

//
struct CustomLookAndFeel : public juce::LookAndFeel_V4
{
    CustomLookAndFeel()
    {
        // base colours
        setColour(juce::ResizableWindow::backgroundColourId, Colors::background);
        setColour(juce::TextButton::buttonColourId, Colors::button);
        setColour(juce::TextButton::buttonOnColourId, Colors::button);
        setColour(juce::TextButton::textColourOffId, Colors::buttonText);
        setColour(juce::TextButton::textColourOnId, Colors::buttonActive);

        // sliders
        setColour(juce::Slider::backgroundColourId, Colors::button);      // track background
        setColour(juce::Slider::trackColourId, Colors::accent);           // filled track pink
        setColour(juce::Slider::thumbColourId, Colors::accent);           // knob pink
        setColour(juce::Slider::textBoxTextColourId, Colors::buttonText); // text
        setColour(juce::Slider::textBoxBackgroundColourId, Colors::panel);// textbox bg
        setColour(juce::Slider::textBoxOutlineColourId, Colors::accent);  // textbox outline
        
    }

    void drawPanelBackground(juce::Graphics& g, juce::Component& comp)
    {
        
        g.fillAll(Colors::panel);
        //g.setColour(Colors::accent);
        g.drawRect(comp.getLocalBounds(), 2);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        auto base = backgroundColour;

        if (shouldDrawButtonAsDown) base = base.darker();
        else if (shouldDrawButtonAsHighlighted) base = base.brighter();

        g.setColour(base);
        g.fillRoundedRectangle(bounds, 6.0f);

        // draw accent outline
        g.setColour(Colors::accent);
        g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
    }

    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override {
        // clear background
		g.fillAll(juce::Colours::transparentBlack); 
		// background
        g.setColour(Colors::panel.withAlpha(0.75f));
        g.fillRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, 6.0f);
		// outline
        g.setColour(Colors::panel);
        g.drawRoundedRectangle(0.5f, 0.5f, (float)width - 1.0f, (float)height - 1.0f, 6.0f, 1.5f);
        //text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f));
        g.drawFittedText(text, 8, 4, width - 16, height - 8, juce::Justification::centredLeft, 2);
    }

    // custom toggle
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto area = button.getLocalBounds().toFloat().reduced(2.0f);
        auto isOn = button.getToggleState();

        // background
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(area, 12.0f);
        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(area, 12.0f, 2.0f);

        // inner switch
        float circleDiameter = area.getHeight() * 0.7f;
        float circleX = isOn ? (area.getRight() - circleDiameter - 6.0f) : (area.getX() + 6.0f);
        juce::Rectangle<float> circle(circleX, area.getY() + (area.getHeight() - circleDiameter) / 2.0f,
            circleDiameter, circleDiameter);

        g.setColour(isOn ? Colors::accentBlue : juce::Colours::darkgrey);
        g.fillEllipse(circle);

        // label text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawFittedText(button.getButtonText(), area.toNearestInt(), juce::Justification::centred, 1);
    }

    //custom dial
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle,
        float rotaryEndAngle, juce::Slider& slider) override {
        // square bounds
        const float side = (float)juce::jmin(width, height);
        juce::Rectangle<float> dial = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height)
            .withSizeKeepingCentre(side, side)
            .reduced(side * 0.12f);           // outer padding

        const float radius = dial.getWidth() * 0.5f;
        const auto  centre = dial.getCentre();

        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // thicknesses
        const float ringThick = juce::jmax(10.0f, radius * 0.18f);       // value ring
        const float outlineThick = juce::jmax(5.0f, radius * 0.02f);    // decorative outline
        const float innerDial = radius * 0.3f;                          //  dial
        const float baseThick = radius * 0.58f;                         // black base thickness

        // base ring
        g.setColour(juce::Colours::black);
        juce::Rectangle<float> base = dial.reduced(radius * 0.2f);
        g.setColour(juce::Colours::black);
        g.fillEllipse(base);

            // pink overlay
            juce::ColourGradient baseGrad(
                Colors::accentPink.withAlpha(0.2f),     // top 
                base.getCentreX(), base.getY(),
                Colors::accentPink.withAlpha(0.05f),    // bottom 
                base.getCentreX(), base.getBottom(),
                false
            );
            g.setGradientFill(baseGrad);
            g.fillEllipse(base);

        // thin black outline
        /*g.setColour(juce::Colours::black);
        g.drawEllipse(base.expanded(outlineThick * 1.5f), outlineThick);*/

        // inner dial
        juce::Rectangle<float> inner = dial.reduced(innerDial);
        g.setColour(Colors::panel);
        g.fillEllipse(inner);
        //gradient
            juce::ColourGradient innerDialGrad(
                Colors::panel,                          // top
                inner.getCentreX(), inner.getY(),
                juce::Colours::black.withAlpha(0.6f),   // bottom
                inner.getCentreX(), inner.getBottom(),
                false
            );
            g.setGradientFill(innerDialGrad);
            g.fillEllipse(inner);

        //inner dial dark overlay
            const float innerOutlineThickness = juce::jmax(0.5f, radius * 0.15f);
            juce::Rectangle<float> innerOutline = inner.reduced(innerOutlineThickness);
            // gradient
            juce::ColourGradient innerGrad(
                juce::Colours::black.withAlpha(0.15f),    // top
                innerOutline.getCentreX(), innerOutline.getY(),
                juce::Colours::black.withAlpha(1.0f),     // bottom 
                innerOutline.getCentreX(), innerOutline.getBottom(),
                false
            );
            g.setGradientFill(innerGrad);
            g.fillEllipse(innerOutline);

        // arrow 
        g.setColour(juce::Colours::white);
        juce::Path pointer;
        float arrowLength = radius * 0.30f;
        float arrowWidth = radius * 0.20f;
        pointer.addTriangle(0.0f, -(radius * 0.70f),-arrowWidth * 0.5f, -(radius * 0.70f - arrowLength), arrowWidth * 0.5f, -(radius * 0.70f - arrowLength) );
        // rotate 
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
        g.fillPath(pointer);

            // inner edge outline  pink gradient
            const float innerEdgeThickness = juce::jmax(0.6f, radius * 0.02f);
            juce::Rectangle<float> innerEdge = innerOutline.reduced(innerOutlineThickness * 1.0f);

            // ring area
            juce::Path pinkBand;
            pinkBand.addEllipse(innerEdge);
            pinkBand.addEllipse(innerEdge.reduced(innerEdgeThickness * 3.0f));
            pinkBand.setUsingNonZeroWinding(false);

            // vertical gradient
            juce::ColourGradient pinkVert(
                Colors::accentPink.withAlpha(0.5f), innerEdge.getCentreX(), innerEdge.getY(),      // top
                Colors::accentPink.withAlpha(0.0f), innerEdge.getCentreX(), innerEdge.getBottom(), // bottom
                false
            );
            g.setGradientFill(pinkVert);
            g.fillPath(pinkBand);

            // pink outline
            g.setColour(Colors::accentPink.withAlpha(0.5f));
            g.drawEllipse(innerEdge, innerEdgeThickness);


        //underneath value ring
        juce::Path baseArc;
        baseArc.addArc(dial.getX(), dial.getY(), dial.getWidth(), dial.getHeight(),rotaryStartAngle, rotaryEndAngle, true);

        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.strokePath(baseArc, juce::PathStrokeType(ringThick, juce::PathStrokeType::curved, juce::PathStrokeType::butt));

        // value ring
        juce::ColourGradient grad(Colors::accentPink, dial.getX(), dial.getY(), Colors::accentTeal, dial.getRight(), dial.getBottom(), false);

        juce::Path valueArc;
        valueArc.addArc(dial.getX(), dial.getY(), dial.getWidth(), dial.getHeight(), rotaryStartAngle, angle, true);

        g.setGradientFill(grad);
        g.strokePath(valueArc, juce::PathStrokeType(ringThick, juce::PathStrokeType::curved, juce::PathStrokeType::butt));

        // tick marks
            g.setColour(juce::Colours::white);
            const int ticks = 30;
            for (int i = 0; i <= ticks; ++i) {
                const float t = (float)i / (float)ticks;
                const float a = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
                const bool majorTick = (i % 5 == 0); // every 5 tick is larger

                // position
                const float ringOffset = (ringThick / radius) * 0.3f; 
                const float innerScale = majorTick ? 1.15f + ringOffset : 1.18f + ringOffset;
                const float outerScale = majorTick ? 1.30f + ringOffset : 1.25f + ringOffset;
                const auto p1 = centre.getPointOnCircumference(radius * innerScale, a);
                const auto p2 = centre.getPointOnCircumference(radius * outerScale, a);
                // line thickness
                const float thickness = majorTick ? juce::jmax(2.5f, radius * 0.018f)
                    : juce::jmax(1.3f, radius * 0.012f);

                g.drawLine({ p1, p2 }, thickness);
            }

            //white value ring caps
            g.setColour(juce::Colours::white);
            const float capThickness = juce::jmax(2.5f, radius * 0.018f); 
            const float capLength = juce::jmax(10.0f, ringThick * 0.5f);  
            // start cap
            auto startP1 = centre.getPointOnCircumference(radius * 0.92f, rotaryStartAngle);
            auto startP2 = centre.getPointOnCircumference(radius * 1.3f, rotaryStartAngle);
            g.drawLine({ startP1, startP2 }, capThickness);
            // end cap
            auto endP1 = centre.getPointOnCircumference(radius * 0.92f, angle);
            auto endP2 = centre.getPointOnCircumference(radius * 1.3f, angle);
            g.drawLine({ endP1, endP2 }, capThickness);

    }
};