/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayPlugInAudioProcessor::DelayPlugInAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    
    addParameter(mDryWetParameter = new juce::AudioParameterFloat("dryWet", "Dry Wet", 0.0f, 1.0f, 0.5f));
    
    addParameter(mFeedbackParameter = new juce::AudioParameterFloat("feedback", "Feedback", 0.0f, 0.98, 0.5f));
    
    addParameter(mDelayTimeParameter = new juce::AudioParameterFloat("delayTime", "Delay Time", 0.1f, MAX_DELAY_TIME, 0.5f));
    
    mCircularBufferLeft = {};
    mCircularBufferRight = {};
    mCircularBufferWriteHead = 0;
    mDelayReadHead = 0;
    mCircularBufferLength = 0;
    mDelayTimeInSamples = 0;
    
    mFeedbackLeft = 0;
    mFeedbackRight = 0;
    
}

DelayPlugInAudioProcessor::~DelayPlugInAudioProcessor()
{
//    if(mCircularBufferLeft != nullptr){
//        delete [] mCircularBufferLeft; //frees memory on the heap for 88200 floats
//        mCircularBufferLeft = nullptr; //reverts to nullptr
//    }
//    if(mCircularBufferRight != nullptr){
//        delete [] mCircularBufferRight; //frees memory on the heap for 88200 floats
//        mCircularBufferRight = nullptr; //reverts to nullptr
//    }
}

//==============================================================================
const juce::String DelayPlugInAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayPlugInAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayPlugInAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayPlugInAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayPlugInAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayPlugInAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayPlugInAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayPlugInAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DelayPlugInAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayPlugInAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DelayPlugInAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mDelayTimeInSamples = sampleRate * *mDelayTimeParameter;
    
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
    
//    if(mCircularBufferLeft == nullptr){
//        mCircularBufferLeft = new float[mCircularBufferLength]; //allocates memory on the heap for 88200 floats
//    }
//
//    if(mCircularBufferRight == nullptr){
//        mCircularBufferRight = new float[mCircularBufferLength]; //allocates memory on the heap for 88200 floats
//    }
    
    mCircularBufferLeft.resize(mCircularBufferLength);
    mCircularBufferRight.resize(mCircularBufferLength);

    mCircularBufferWriteHead = 0;
    
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void DelayPlugInAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayPlugInAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void DelayPlugInAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    mDelayTimeInSamples = getSampleRate() * *mDelayTimeParameter;

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    float* leftChannel = buffer.getWritePointer(0); //Returns a writeable pointer to the buffer's left channel
    float* rightChannel = buffer.getWritePointer(1); //Returns a writeable pointer to the buffer's right channel
    
    auto samples = buffer.getNumSamples();
    
    //loops through the samples in the buffer
    for(int i = 0; i < samples; i++){
        
        mCircularBufferLeft.at(mCircularBufferWriteHead) = leftChannel[i] + mFeedbackLeft;
        mCircularBufferRight.at(mCircularBufferWriteHead) = rightChannel[i] + mFeedbackRight;

        mDelayReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;

        if(mDelayReadHead < 0){
           mDelayReadHead += mCircularBufferLength;
        }

        float delaySampleLeft = mCircularBufferLeft[(int)mDelayReadHead]; //output signal
        float delaySampleRight = mCircularBufferRight[(int)mDelayReadHead]; //output signal

        mFeedbackLeft = delaySampleLeft * *mFeedbackParameter;
        mFeedbackRight = delaySampleRight * *mFeedbackParameter;
        
        mCircularBufferWriteHead++;
        
        buffer.setSample(0, i, buffer.getSample(0, i) * (1 - *mDryWetParameter) + delaySampleLeft * *mDryWetParameter);
        buffer.setSample(1, i, buffer.getSample(1, i) * (1 - *mDryWetParameter) + delaySampleRight * *mDryWetParameter);

        if(mCircularBufferWriteHead == mCircularBufferLength){
           mCircularBufferWriteHead = 0;
        }
    }
}

//==============================================================================
bool DelayPlugInAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DelayPlugInAudioProcessor::createEditor()
{
    return new DelayPlugInAudioProcessorEditor (*this);
}

//==============================================================================
void DelayPlugInAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DelayPlugInAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayPlugInAudioProcessor();
}
