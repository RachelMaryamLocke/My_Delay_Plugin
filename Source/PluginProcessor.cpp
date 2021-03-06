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
    
    addParameter(mDryWetParameter = new juce::AudioParameterFloat("dryWet", "Dry Wet", 0.0f, 1.0f, 0.0f));
    
    addParameter(mFeedbackParameter = new juce::AudioParameterFloat("feedback", "Feedback", 0.0f, 0.98, 0.0f));
    
    addParameter(mDelayTimeParameter = new juce::AudioParameterFloat("delayTime", "Delay Time", 0.1f, MAX_DELAY_TIME, 0.1f));
        
    mDelayTimeSmoothed = 0;
    mCircularBufferWriteHead = 0;
    mDelayReadHead = 0;
    mCircularBufferLength = 0;
    mDelayTimeInSamples = 0;
    
    mFeedbackLeft = 0;
    mFeedbackRight = 0;
    
}

DelayPlugInAudioProcessor::~DelayPlugInAudioProcessor()
{
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
    
    mCircularBufferLeft.resize(mCircularBufferLength);
    mCircularBufferRight.resize(mCircularBufferLength);

    mCircularBufferWriteHead = 0;
    
    mDelayTimeSmoothed = *mDelayTimeParameter;
    
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
    
    float* leftChannel = buffer.getWritePointer(0); //Returns a writeable pointer to the buffer's left channel
    float* rightChannel = buffer.getWritePointer(1); //Returns a writeable pointer to the buffer's right channel
    
    auto samples = buffer.getNumSamples();
    
    //loops through the samples in the buffer
    for(int i = 0; i < samples; i++){
        
        mDelayTimeSmoothed = mDelayTimeSmoothed - 0.001 * (mDelayTimeSmoothed - *mDelayTimeParameter);
        mDelayTimeInSamples = getSampleRate() * mDelayTimeSmoothed;
        
        mCircularBufferLeft.at(mCircularBufferWriteHead) = leftChannel[i] + mFeedbackLeft;
        mCircularBufferRight.at(mCircularBufferWriteHead) = rightChannel[i] + mFeedbackRight;

        mDelayReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;

        if(mDelayReadHead < 0){
           mDelayReadHead += mCircularBufferLength;
        }
        
        //linear interpolation code
        
        int readHead_x = (int)mDelayReadHead;
        int readHead_x1 = readHead_x + 1;
        float readHeadFloat = mDelayReadHead - readHead_x;

        if(readHead_x1 >= mCircularBufferLength){
            readHead_x1 -= mCircularBufferLength;
        }

        float delaySampleLeft = linearInterp(mCircularBufferLeft.at(readHead_x), mCircularBufferLeft.at(readHead_x1), readHeadFloat); //output signal
        float delaySampleRight = linearInterp(mCircularBufferRight.at(readHead_x), mCircularBufferRight.at(readHead_x1), readHeadFloat);

//        float delaySampleLeft = mCircularBufferLeft[(int)mDelayReadHead]; //output signal
//        float delaySampleRight = mCircularBufferRight[(int)mDelayReadHead]; //output signal

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

float DelayPlugInAudioProcessor::linearInterp(float sample_x, float sample_x1, float in_phase){
    return (1 - in_phase) * sample_x + in_phase * sample_x1;
}
