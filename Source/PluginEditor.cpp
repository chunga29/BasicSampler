/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicSamplerAudioProcessorEditor::BasicSamplerAudioProcessorEditor (BasicSamplerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    mLoadButton.onClick = [&]() { audioProcessor.loadFile(); };
    
    addAndMakeVisible(mLoadButton);
    
    setSize (600, 200);
}

BasicSamplerAudioProcessorEditor::~BasicSamplerAudioProcessorEditor()
{
}

//==============================================================================
void BasicSamplerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);
    g.setColour(juce::Colours::white);
    
    if (mShouldBePainting) {
        juce::Path p;
        mAudioPoints.clear();
        
        auto waveform = audioProcessor.getWaveForm();
        auto ratio = waveform.getNumSamples() / getWidth();
        auto buffer = waveform.getReadPointer(0);
        
        // Scale audio file to window on x axis
        for (int sample = 0; sample < waveform.getNumSamples(); sample += ratio) {
            mAudioPoints.push_back(buffer[sample]);
        }
        
        p.startNewSubPath(0, getHeight() / 2);
        
        // Scale audio on y axis
        for (int sample = 0; sample < mAudioPoints.size(); ++sample) {
            auto point = juce::jmap<float>(mAudioPoints[sample], -1.f, 1.f, 200, 0);
            p.lineTo(sample, point);
        }
        
        g.strokePath(p, juce::PathStrokeType(2));
        
        mShouldBePainting = false;
    }
}

void BasicSamplerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    // mLoadButton.setBounds(getWidth()/2 - 50, getHeight()/2 - 50, 100, 100);
}

bool BasicSamplerAudioProcessorEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto file : files) {
        if (file.contains(".wav") || file.contains(".mp3") || file.contains(".aif")) {
            return true;
        }
    }
    
    return false;
}

void BasicSamplerAudioProcessorEditor::filesDropped(const juce::StringArray& files, int x, int y)
{
    for (auto file : files) {
        if (isInterestedInFileDrag(file)) {
            mShouldBePainting = true;
            audioProcessor.loadFile(file);
        }
    }
    repaint();
}
