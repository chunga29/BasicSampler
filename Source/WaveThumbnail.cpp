/*
  ==============================================================================

    WaveThumbnail.cpp
    Created: 4 Feb 2024 6:14:19pm
    Author:  Adam Chung

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WaveThumbnail.h"

//==============================================================================
WaveThumbnail::WaveThumbnail(BasicSamplerAudioProcessor& p) : audioProcessor(p)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

WaveThumbnail::~WaveThumbnail()
{
}

void WaveThumbnail::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::cadetblue.darker());
    
    auto waveform = audioProcessor.getWaveForm();
    
    if (waveform.getNumSamples() > 0) {
        juce::Path p;
        mAudioPoints.clear();
        
        
        auto ratio = waveform.getNumSamples() / getWidth();
        auto buffer = waveform.getReadPointer(0);
        
        // Scale audio file to window on x axis
        for (int sample = 0; sample < waveform.getNumSamples(); sample += ratio) {
            mAudioPoints.push_back(buffer[sample]);
        }
        
        g.setColour(juce::Colours::yellow);
        p.startNewSubPath(0, getHeight() / 2);
        
        // Scale audio on y axis
        for (int sample = 0; sample < mAudioPoints.size(); ++sample) {
            auto point = juce::jmap<float>(mAudioPoints[sample], -1.f, 1.f, getHeight(), 0);
            p.lineTo(sample, point);
        }
        
        g.strokePath(p, juce::PathStrokeType(2));
        
        g.setColour(juce::Colours::white);
        g.setFont(15.f);
        auto textBounds = getLocalBounds().reduced(10, 10);
        g.drawFittedText(mFileName, textBounds, juce::Justification::topRight, 1);

        auto playHeadPosition = juce::jmap<int>(audioProcessor.getSampleCount(), 0, audioProcessor.getWaveForm().getNumSamples(), 0, getWidth());
        
        g.setColour(juce::Colours::white);
        g.drawLine(playHeadPosition, 0, playHeadPosition, getHeight(), 2.f);
        
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillRect(0, 0, playHeadPosition, getHeight());
    } else {
        g.setColour(juce::Colours::white);
        g.setFont(40.f);
        g.drawFittedText("Drop an Audio File to Load", getLocalBounds(), juce::Justification::centred, 1);
    }
}

void WaveThumbnail::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

bool WaveThumbnail::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto file : files) {
        if (file.contains(".wav") || file.contains(".mp3") || file.contains(".aif")) {
            return true;
        }
    }
    
    return false;
}

void WaveThumbnail::filesDropped(const juce::StringArray& files, int x, int y)
{
    for (auto file : files) {
        if (isInterestedInFileDrag(file)) {
            auto myFile = std::make_unique<juce::File> (file);
            mFileName = myFile->getFileNameWithoutExtension();
            audioProcessor.loadFile(file);
        }
    }
    repaint();
}
