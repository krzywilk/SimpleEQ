/*
  ==============================================================================

    SingleChannelFFTPathGenerator.cpp
    Created: 25 Feb 2023 10:15:54pm
    Author:  digica_kwolk

  ==============================================================================
*/

#include "SingleChannelFFTPathGenerator.h"

SingleChannelFFTPathGenerator::SingleChannelFFTPathGenerator(SingleChannelBlockFifoBuffer<SimpleEQAudioProcessor::BlockType>& scsf) :
    singleChannelFifo(&scsf)
{
    singleChannelFFTDataGenerator.changeOrder(singleChannelFFTDataGenerator.FFTOrder::order2048);
    monoBuffer.setSize(1, singleChannelFFTDataGenerator.getFFTSize());
}
void SingleChannelFFTPathGenerator::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;
    while (singleChannelFifo->getNumCompleteBuffersAvailable() > 0)
    {
        if (singleChannelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            auto size = tempIncomingBuffer.getNumSamples();

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
                monoBuffer.getReadPointer(0, size),
                monoBuffer.getNumSamples() - size);

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                tempIncomingBuffer.getReadPointer(0, 0),
                size);

            singleChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }

    const auto fftSize = singleChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate / double(fftSize);

    while (singleChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        if (singleChannelFFTDataGenerator.getFFTData(fftData))
        {
            FFTData2PathConverter.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
        }
    }

    while (FFTData2PathConverter.getNumPathsAvailable() > 0)
    {
        FFTData2PathConverter.getPath(singleChannelFFTPath);
    }
}
