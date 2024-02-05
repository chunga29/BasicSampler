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
    auto psycheImage = juce::ImageCache::getFromMemory(BinaryData::PsycheCOVERHalfReso_png, BinaryData::PsycheCOVERHalfReso_pngSize);
    
    if (!psycheImage.isNull()) {
        mImageComponent.setImage(psycheImage, juce::RectanglePlacement::xLeft);
    } else {
        jassert(!psycheImage.isNull());
    }
    
    addAndMakeVisible(mWaveThumbnail);
    addAndMakeVisible(mADSR);
    addAndMakeVisible(mImageComponent);
    
    startTimerHz(30);
    
    setSize (800, 600);
}

BasicSamplerAudioProcessorEditor::~BasicSamplerAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void BasicSamplerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::white);
}

void BasicSamplerAudioProcessorEditor::resized()
{
    mWaveThumbnail.setBoundsRelative(0.0f, 0.25f, 1.0f, 0.5);
    mADSR.setBoundsRelative(0.0f, 0.75f, 1.0f, 0.25f);
    mImageComponent.setBoundsRelative(0.f, 0.f, 0.25f, 0.25f);
}

void BasicSamplerAudioProcessorEditor::timerCallback()
{
    repaint();
}
