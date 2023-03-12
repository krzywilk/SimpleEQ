/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor
	(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	),
	chainFiltersFactory(getSampleRate())
#endif
{
}

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
	return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName(int index)
{
	return {};
}

void SimpleEQAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	// Use this method as the place to do any pre-playback
	// initialisation that you need..

	juce::dsp::ProcessSpec spec;

	spec.maximumBlockSize = samplesPerBlock;

	spec.numChannels = 1;

	spec.sampleRate = sampleRate;

	chainFiltersFactory.setSampleRate(sampleRate);

	leftChain.prepare(spec);
	rightChain.prepare(spec);

	updateFilters();

	leftChannelProcessedBlockFifo.prepare(samplesPerBlock);
	rightChannelProcessedBlockFifo.prepare(samplesPerBlock);

	osc.initialise([](float x) { return std::sin(x); });

	spec.numChannels = getTotalNumOutputChannels();
	osc.prepare(spec);
	osc.setFrequency(440);
}

void SimpleEQAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (//layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
		layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void SimpleEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	// In case we have more outputs than inputs, this code clears any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	// This is here to avoid people getting screaming feedback
	// when they first compile a plugin, but obviously you don't need to keep
	// this code if your algorithm always overwrites all the output channels.
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	updateFilters();

	juce::dsp::AudioBlock<float> block(buffer);

	//    buffer.clear();
	//
	//    for( int i = 0; i < buffer.getNumSamples(); ++i )
	//    {
	//        buffer.setSample(0, i, osc.processSample(0));
	//    }
	//
	//    juce::dsp::ProcessContextReplacing<float> stereoContext(block);
	//    osc.process(stereoContext);

	auto leftBlock = block.getSingleChannelBlock(0);
	auto rightBlock = block.getSingleChannelBlock(1);

	juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
	juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

	leftChain.process(leftContext);
	rightChain.process(rightContext);

	leftChannelProcessedBlockFifo.update(buffer);
	rightChannelProcessedBlockFifo.update(buffer);
}

//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
	return new SimpleEQAudioProcessorEditor(*this);
	//    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.

	juce::MemoryOutputStream mos(destData, true);
	apvts.state.writeToStream(mos);
}

void SimpleEQAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
	auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
	if (tree.isValid())
	{
		apvts.replaceState(tree);
		updateFilters();
	}
}

ChainConfiguration getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
	ChainConfiguration settings;

	settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
	settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
	settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
	settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
	settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
	settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
	settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

	settings.lowCutBypassed = apvts.getRawParameterValue("LowCut Bypassed")->load() > 0.5f;
	settings.peakBypassed = apvts.getRawParameterValue("Peak Bypassed")->load() > 0.5f;
	settings.highCutBypassed = apvts.getRawParameterValue("HighCut Bypassed")->load() > 0.5f;

	return settings;
}

void SimpleEQAudioProcessor::updatePeakFilter(const ChainConfiguration& chainConfiguration)
{
	auto peakCoefficients = chainFiltersFactory.makePeakFilter(chainConfiguration);

	leftChain.setBypassed<ChainPositions::Peak>(chainConfiguration.peakBypassed);
	rightChain.setBypassed<ChainPositions::Peak>(chainConfiguration.peakBypassed);

	*leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
	*rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
}


void SimpleEQAudioProcessor::updateLowCutFilters(const ChainConfiguration& chainConfiguration)
{
	auto cutCoefficients = chainFiltersFactory.makeLowCutFilter(chainConfiguration);
	auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
	auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

	leftChain.setBypassed<ChainPositions::LowCut>(chainConfiguration.lowCutBypassed);
	rightChain.setBypassed<ChainPositions::LowCut>(chainConfiguration.lowCutBypassed);

	updateCutFilter(rightLowCut, cutCoefficients, chainConfiguration.lowCutSlope);
	updateCutFilter(leftLowCut, cutCoefficients, chainConfiguration.lowCutSlope);
}

void SimpleEQAudioProcessor::updateHighCutFilters(const ChainConfiguration& chainConfiguration)
{
	auto highCutCoefficients = chainFiltersFactory.makeHighCutFilter(chainConfiguration);

	auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
	auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();

	leftChain.setBypassed<ChainPositions::HighCut>(chainConfiguration.highCutBypassed);
	rightChain.setBypassed<ChainPositions::HighCut>(chainConfiguration.highCutBypassed);

	updateCutFilter(leftHighCut, highCutCoefficients, chainConfiguration.highCutSlope);
	updateCutFilter(rightHighCut, highCutCoefficients, chainConfiguration.highCutSlope);
}

void SimpleEQAudioProcessor::updateFilters()
{
	auto chainSettings = getChainSettings(apvts);

	updateLowCutFilters(chainSettings);
	updatePeakFilter(chainSettings);
	updateHighCutFilters(chainSettings);
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleEQAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
		"LowCut Freq",
		juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
		20.f));

	layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
		"HighCut Freq",
		juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
		20000.f));

	layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq",
		"Peak Freq",
		juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
		750.f));

	layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain",
		"Peak Gain",
		juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
		0.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality",
		"Peak Quality",
		juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
		1.f));

	juce::StringArray stringArray;
	for (int i = 0; i < 4; ++i)
	{
		juce::String str;
		str << (12 + i * 12);
		str << " db/Oct";
		stringArray.add(str);
	}

	layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
	layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));

	layout.add(std::make_unique<juce::AudioParameterBool>("LowCut Bypassed", "LowCut Bypassed", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("Peak Bypassed", "Peak Bypassed", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("HighCut Bypassed", "HighCut Bypassed", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("Analyzer Enabled", "Analyzer Enabled", true));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new SimpleEQAudioProcessor();
}

