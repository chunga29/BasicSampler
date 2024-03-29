/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicSamplerAudioProcessor::BasicSamplerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), mAPVTS(*this, nullptr, "PARAMETERS", createParameters())
#endif
{
    mFormatManager.registerBasicFormats();
    mAPVTS.state.addListener(this);
    
    for (int i = 0; i < mNumVoices; i++) {
        mSampler.addVoice(new juce::SamplerVoice());
    }
}

BasicSamplerAudioProcessor::~BasicSamplerAudioProcessor()
{
    delete mFormatReader;
}

//==============================================================================
const juce::String BasicSamplerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicSamplerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicSamplerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicSamplerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicSamplerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicSamplerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicSamplerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicSamplerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicSamplerAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicSamplerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicSamplerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mSampler.setCurrentPlaybackSampleRate(sampleRate);
    updateADSR();
}

void BasicSamplerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicSamplerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void BasicSamplerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    if (mShouldUpdate) {
        updateADSR();
    }
    
    juce::MidiMessage m;
    juce::MidiBuffer::Iterator it { midiMessages };
    int sample;
    
    while (it.getNextEvent(m, sample)) {
        if (m.isNoteOn()) {
            mIsNotePlayed = true;
        } else if (m.isNoteOff()) {
            mIsNotePlayed = false;
        }
    }
    
    mSampleCount = mIsNotePlayed ? mSampleCount += buffer.getNumSamples() : 0;

    mSampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool BasicSamplerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicSamplerAudioProcessor::createEditor()
{
    return new BasicSamplerAudioProcessorEditor (*this);
}

//==============================================================================
void BasicSamplerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BasicSamplerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void BasicSamplerAudioProcessor::loadFile()
{
    using namespace juce;
    mSampler.clearSounds();
    FileChooser chooser { "Please load a file" };
    if (chooser.browseForFileToOpen()) {
        auto file = chooser.getResult();
        mFormatReader = mFormatManager.createReaderFor(file);
    }
    
    BigInteger range;
    range.setRange(0, 128, true);
    
    mSampler.addSound(new SamplerSound("Sample", *mFormatReader, range, 60, 0.0, 0.1, 10.0));
}

void BasicSamplerAudioProcessor::loadFile(const juce::String &path)
{
    using namespace juce;
    mSampler.clearSounds();
    auto file = File (path);
    mFormatReader = mFormatManager.createReaderFor(file);
    
    auto sampleLength = static_cast<int>(mFormatReader->lengthInSamples);
    
    mWaveForm.setSize(1, sampleLength);
    mFormatReader->read(&mWaveForm, 0, sampleLength, 0, true, false);
    
    BigInteger range;
    range.setRange(0, 128, true);
    
    mSampler.addSound(new SamplerSound("Sample", *mFormatReader, range, 60, 0.0, 0.1, 10.0));
    
    updateADSR();
}

void BasicSamplerAudioProcessor::updateADSR()
{
    mADSRParams.attack = mAPVTS.getRawParameterValue("ATTACK")->load();
    mADSRParams.decay = mAPVTS.getRawParameterValue("DECAY")->load();
    mADSRParams.sustain = mAPVTS.getRawParameterValue("SUSTAIN")->load();
    mADSRParams.release = mAPVTS.getRawParameterValue("RELEASE")->load();
    
    for (int i = 0; i < mSampler.getNumSounds(); ++i) {
        if (auto sound = dynamic_cast<juce::SamplerSound*>(mSampler.getSound(i).get())) {
            sound->setEnvelopeParameters(mADSRParams);
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout BasicSamplerAudioProcessor::createParameters() 
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "ATTACK", 1 }, "Attack", 0.0f, 5.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "DECAY", 1 }, "Decay", 0.0f, 3.0f, 2.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "SUSTAIN", 1 }, "Sustain", 0.0f, 1.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "RELEASE", 1 }, "Release", 0.0f, 5.0f, 0.75f));
    
    return { parameters.begin(), parameters.end() };
}

void BasicSamplerAudioProcessor::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged, const juce::Identifier &property)
{
    mShouldUpdate = true;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicSamplerAudioProcessor();
}
