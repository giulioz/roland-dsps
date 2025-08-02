/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include <JuceHeader.h>

//==============================================================================
/**
 */
class LSP_juceAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     juce::ChangeListener {
public:
  LSP_juceAudioProcessorEditor(LSP_juceAudioProcessor &);
  ~LSP_juceAudioProcessorEditor() override;

  //==============================================================================
  void resized() override;

private:
  LSP_juceAudioProcessor &audioProcessor;

  juce::TextEditor pgmInput;
  juce::TextButton genButton;

  void changeListenerCallback(juce::ChangeBroadcaster *source);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LSP_juceAudioProcessorEditor)
};
