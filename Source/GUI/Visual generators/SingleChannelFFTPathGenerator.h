/*
  ==============================================================================

    SingleChannelFFTPathGenerator.h
    Created: 25 Feb 2023 10:15:54pm
    Author:  digica_kwolk

  ==============================================================================
*/

#pragma once
#include "../../PluginProcessor.h"
#include "../Source/Data processing/mathGenerators.h"
#include "../Source/Data processing/converters.h"


class SingleChannelFFTPathGenerator
{
public:
    SingleChannelFFTPathGenerator(SingleChannelSampleFifo<SimpleEQAudioProcessor::BlockType>& scsf);
    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath() { return singleChannelFFTPath; }
private:
    SingleChannelSampleFifo<SimpleEQAudioProcessor::BlockType>* singleChannelFifo;

    juce::AudioBuffer<float> monoBuffer;

    FFTDataGenerator<std::vector<float>> singleChannelFFTDataGenerator;

    AnalyzerPathConverter<juce::Path> FFTData2PathConverter;

    juce::Path singleChannelFFTPath;
};
