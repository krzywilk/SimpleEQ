/*
  ==============================================================================

    ResponseStereoSpikedCurveComponent.cpp
    Created: 26 Feb 2023 5:19:32pm
    Author:  digica_kwolk

  ==============================================================================
*/

#include <JuceHeader.h>
#include "../Source/PluginProcessor.h"
#include "../Visual generators/SingleChannelFFTPathGenerator.h"


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

