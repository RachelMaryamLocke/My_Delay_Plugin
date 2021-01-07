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
    
    mDryWetSlider.setBounds(0, 0, 200, 100);
    mDryWetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDryWetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
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
    
//    addAndMakeVisible(mDryWetLabel);
//    mDryWetLabel.setText ("Dry/Wet", juce::dontSendNotification);
//    mDryWetLabel.attachToComponent(&mDryWetSlider, true);
    
    //==============================================================================
    
    //Feedback control
    
    juce::AudioParameterFloat* feedbackParameter = ((juce::AudioParameterFloat*)params.getUnchecked(1));

    mFeedbackSlider.setBounds(100, 0, 200, 100);
    mFeedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mFeedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mFeedbackSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end);
    mDryWetSlider.setValue(*feedbackParameter);
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
}

DelayPlugInAudioProcessorEditor::~DelayPlugInAudioProcessorEditor()
{
}

//==============================================================================
void DelayPlugInAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("", getLocalBounds(), juce::Justification::centred, 1);
}

void DelayPlugInAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
