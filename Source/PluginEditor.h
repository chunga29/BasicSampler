/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class BasicSamplerAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                          public juce::FileDragAndDropTarget
{
public:
    BasicSamplerAudioProcessorEditor (BasicSamplerAudioProcessor&);
    ~BasicSamplerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    

private:
    juce::TextButton mLoadButton { "Load " };
    std::vector<float> mAudioPoints;
    bool mShouldBePainting { false };
    
    juce::Slider mAttackSlider,
                 mDecaySlider,
                 mSustainSlider,
                 mReleaseSlider;
    
    juce::Label mAttackLabel,
                mDecayLabel,
                mSustainLabel,
                mReleaseLabel;
    
    BasicSamplerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicSamplerAudioProcessorEditor)
};
