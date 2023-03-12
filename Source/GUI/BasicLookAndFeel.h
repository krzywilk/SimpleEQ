 /*
  ==============================================================================

    BasicLookAndFeel.h
    Created: 24 Feb 2023 10:37:31pm
    Author:  digica_kwolk

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


struct BasicLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider(juce::Graphics&,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;

    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& toggleButton,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;
};