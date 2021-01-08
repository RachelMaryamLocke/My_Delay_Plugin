/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

#define MAX_DELAY_TIME 2

//==============================================================================
/**
*/
class DelayPlugInAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DelayPlugInAudioProcessor();
    ~DelayPlugInAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    float linearInterp(float sample_x, float sample_x1, float in_phase);

private:
    
    juce::AudioParameterFloat* mDryWetParameter;
    juce::AudioParameterFloat* mFeedbackParameter;
    juce::AudioParameterFloat* mDelayTimeParameter;
    
    float mFeedbackLeft;
    float mFeedbackRight;
    
    float mDelayTimeInSamples;
    float mDelayReadHead; //reads audio samples from circular buffer
    
    int mCircularBufferWriteHead; //writes audio samples to circular buffer
    int mCircularBufferLength;
    
    std::vector<float> mCircularBufferLeft;
    std::vector<float> mCircularBufferRight;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayPlugInAudioProcessor)
};
