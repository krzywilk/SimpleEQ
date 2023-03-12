/*
  ==============================================================================

    CutPeakFiltersFactory.h
    Created: 11 Mar 2023 8:16:09pm
    Author:  digica_kwolk

  ==============================================================================
*/
#pragma once
#include "JuceHeader.h"
#include "../../Configuration management/Chain/ChainConfiguration.h"


struct ChainFiltersFactory
{
    using Coefficients = juce::dsp::IIR::Filter<float>::CoefficientsPtr;
    ChainFiltersFactory(double sampleRate)
    {
        this->sampleRate = sampleRate; };
        auto makeLowCutFilter(const ChainConfiguration& chainSettings)
        {
            return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                sampleRate,
                2 * (chainSettings.lowCutSlope + 1));
        }


        auto makeHighCutFilter(const ChainConfiguration& chainSettings)
        {
            return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
                sampleRate,
                2 * (chainSettings.highCutSlope + 1));
        }

        Coefficients makePeakFilter(const ChainConfiguration& chainConfiguration)
        {
            return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                chainConfiguration.peakFreq,
                chainConfiguration.peakQuality,
                juce::Decibels::decibelsToGain(chainConfiguration.peakGainInDecibels));
        }

        void setSampleRate(double sampleRate) { this->sampleRate = sampleRate; };
    private:
        double sampleRate;
};

