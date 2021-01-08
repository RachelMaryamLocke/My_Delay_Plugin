/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayPlugInAudioProcessorEditor::DelayPlugInAudioProcessorEditor (DelayPlugInAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    auto& params = processor.getParameters(); //Reference to the parameters
    
    //DryWet Control
    
    juce::AudioParameterFloat* dryWetParameter = ((juce::AudioParameterFloat*)params.getUnchecked(0));
    
    mDryWetSlider.setBounds(100, 0, 200, 100);
    mDryWetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDryWetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDryWetSlider.setColour(juce::Slider::thumbColourId, juce::Colour(219,254,25));
    mDryWetSlider.setValue(*dryWetParameter);
    mDryWetSlider.setRange(dryWetParameter->range.start, dryWetParameter->range.end);
    addAndMakeVisible(mDryWetSlider);
    
    mDryWetSlider.onValueChange = [this, dryWetParameter]
    {
        *dryWetParameter = mDryWetSlider.getValue();
    };
    
    mDryWetSlider.onDragStart = [dryWetParameter]
    {
        dryWetParameter->beginChangeGesture();
    };
    
    mDryWetSlider.onDragEnd = [dryWetParameter]
    {
        dryWetParameter->endChangeGesture();
    };
    
    addAndMakeVisible(mDryWetLabel);
    mDryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    mDryWetLabel.attachToComponent(&mDryWetSlider, true);
    mDryWetLabel.setColour(juce::Label::textColourId, juce::Colour(219,254,25));
    
    //==============================================================================
    
    //Feedback control
    
    juce::AudioParameterFloat* feedbackParameter = ((juce::AudioParameterFloat*)params.getUnchecked(1));

    mFeedbackSlider.setBounds(100, 100, 200, 100);
    mFeedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mFeedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mFeedbackSlider.setColour(juce::Slider::thumbColourId, juce::Colour(219,254,25));
    mFeedbackSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end);
    mFeedbackSlider.setValue(*feedbackParameter);
    addAndMakeVisible(mFeedbackSlider);

    mFeedbackSlider.onValueChange = [this, feedbackParameter]
    {
        *feedbackParameter= mFeedbackSlider.getValue();
    };

    mFeedbackSlider.onDragStart = [feedbackParameter]
    {
        feedbackParameter->beginChangeGesture();
    };

    mFeedbackSlider.onDragEnd = [feedbackParameter]
    {
        feedbackParameter->endChangeGesture();
    };

    addAndMakeVisible(mFeedbackLabel);
    mFeedbackLabel.setText("Feedback", juce::dontSendNotification);
    mFeedbackLabel.attachToComponent(&mFeedbackSlider, true);
    mFeedbackLabel.setColour(juce::Label::textColourId, juce::Colour(219,254,25));
    
    //==============================================================================
    
    //Delay time control
    
    juce::AudioParameterFloat* delayTimeParameter = ((juce::AudioParameterFloat*)params.getUnchecked(2));

    mDelayTimeSlider.setBounds(100, 200, 200, 100);
    mDelayTimeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDelayTimeSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDelayTimeSlider.setColour(juce::Slider::thumbColourId, juce::Colour(219,254,25));
    mDelayTimeSlider.setRange(delayTimeParameter->range.start, delayTimeParameter->range.end);
    mDelayTimeSlider.setValue(*delayTimeParameter);
    addAndMakeVisible(mDelayTimeSlider);

    mDelayTimeSlider.onValueChange = [this, delayTimeParameter]
    {
        *delayTimeParameter= mDelayTimeSlider.getValue();
    };

    mDelayTimeSlider.onDragStart = [delayTimeParameter]
    {
        delayTimeParameter->beginChangeGesture();
    };

    mDelayTimeSlider.onDragEnd = [delayTimeParameter]
    {
        delayTimeParameter->endChangeGesture();
    };

    addAndMakeVisible(mDelayTimeLabel);
    mDelayTimeLabel.setText("Delay Time", juce::dontSendNotification);
    mDelayTimeLabel.attachToComponent(&mDelayTimeSlider, true);
    mDelayTimeLabel.setColour(juce::Label::textColourId, juce::Colour(219,254,25));
    
    
}

DelayPlugInAudioProcessorEditor::~DelayPlugInAudioProcessorEditor()
{
}

//==============================================================================
void DelayPlugInAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::darkslategrey);
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("", getLocalBounds(), juce::Justification::centred, 1);
}

void DelayPlugInAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

}
