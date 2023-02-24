/*
  ==============================================================================

    visuals_generators.h
    Created: 22 Feb 2023 9:17:02pm
    Author:  digica_kwolk

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginEditor.h"
#include "mathGenerators.h"
#include "converters.h"

struct SingleChannelFFTPathGenerator
{
    SingleChannelFFTPathGenerator(SingleChannelSampleFifo<SimpleEQAudioProcessor::BlockType>& scsf) :
        singleChannelFifo(&scsf)
    {
        singleChannelFFTDataGenerator.changeOrder(FFTOrder::order2048);
        monoBuffer.setSize(1, singleChannelFFTDataGenerator.getFFTSize());
    }
    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath() { return singleChannelFFTPath; }
private:
    SingleChannelSampleFifo<SimpleEQAudioProcessor::BlockType>* singleChannelFifo;

    juce::AudioBuffer<float> monoBuffer;

    FFTDataGenerator<std::vector<float>> singleChannelFFTDataGenerator;

    AnalyzerPathConverter<juce::Path> FFTData2PathConverter;

    juce::Path singleChannelFFTPath;
};


    struct ResponseCurveComponent : juce::Component,
        juce::AudioProcessorParameter::Listener,
        juce::Timer
    {
        ResponseCurveComponent(SimpleEQAudioProcessor&);
        ~ResponseCurveComponent();

        void parameterValueChanged(int parameterIndex, float newValue) override;

        void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }

        void timerCallback() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void toggleAnalysisEnablement(bool enabled)
        {
            shouldShowFFTAnalysis = enabled;
        }
    private:
        SimpleEQAudioProcessor& audioProcessor;

        bool shouldShowFFTAnalysis = true;

        juce::Atomic<bool> parametersChanged{ false };

        MonoChain monoChain;

        void updateResponseCurve();

        juce::Path responseCurve;

        void updateChain();

        void drawBackgroundGrid(juce::Graphics& g);
        void drawTextLabels(juce::Graphics& g);

        std::vector<float> getFrequencies();
        std::vector<float> getGains();
        std::vector<float> getXs(const std::vector<float>& freqs, float left, float width);

        juce::Rectangle<int> getRenderArea();

        juce::Rectangle<int> getAnalysisArea();

        SingleChannelFFTPathGenerator leftPathProducer, rightPathProducer;
    };
