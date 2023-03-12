/*
  ==============================================================================

    ChanConfiguration.h
    Created: 12 Mar 2023 8:15:55pm
    Author:  digica_kwolk

  ==============================================================================
*/

#pragma once
enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

struct ChainConfiguration
{
    float peakFreq{ 0 }, peakGainInDecibels{ 0 }, peakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };

    Slope lowCutSlope{ Slope::Slope_12 }, highCutSlope{ Slope::Slope_12 };

    bool lowCutBypassed{ false }, peakBypassed{ false }, highCutBypassed{ false };
};