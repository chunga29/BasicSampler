/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicSamplerAudioProcessorEditor::BasicSamplerAudioProcessorEditor (BasicSamplerAudioProcessor& p)
    : AudioProcessorEditor (&p), mWaveThumbnail(p), mADSR(p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(mWaveThumbnail);
    addAndMakeVisible(mADSR);
    
    startTimerHz(30);
    
    setSize (600, 400);
}

BasicSamplerAudioProcessorEditor::~BasicSamplerAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void BasicSamplerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);
}

void BasicSamplerAudioProcessorEditor::resized()
{
    mWaveThumbnail.setBoundsRelative(0.0f, 0.25f, 1.0f, 0.5);
    mADSR.setBoundsRelative(0.0f, 0.75f, 1.0f, 0.25f);
}

void BasicSamplerAudioProcessorEditor::timerCallback()
{
    repaint();
}
