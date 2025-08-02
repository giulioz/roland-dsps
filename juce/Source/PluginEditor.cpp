/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
LSP_juceAudioProcessorEditor::LSP_juceAudioProcessorEditor(
    LSP_juceAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setSize(820, 300);

  addAndMakeVisible(pgmInput);
  pgmInput.setText("type something...");
  pgmInput.setMultiLine(false);

  addAndMakeVisible(genButton);
  genButton.setButtonText("GEN");
  genButton.onClick = [this] {
    juce::String str = pgmInput.getText();

    uint8_t all_params[23] = {0};
    for (int i = 0; i < str.length() && i < 23 * 2; i += 2) {
      if (i + 1 < str.length()) {
        all_params[i / 2] = (uint8_t)str.substring(i, i + 2).getHexValue32();
      }
    }

    audioProcessor.regenProgram(all_params[0], all_params[1], &all_params[2]);
  };

  p.addChangeListener(this);
}

LSP_juceAudioProcessorEditor::~LSP_juceAudioProcessorEditor() {}

void LSP_juceAudioProcessorEditor::resized() {
  auto area = getLocalBounds().reduced(8);
  auto buttonWidth = 60;
  pgmInput.setBounds(
      area.removeFromLeft(area.getWidth() - buttonWidth).reduced(0, 2));
  genButton.setBounds(area);
}

void LSP_juceAudioProcessorEditor::changeListenerCallback(
    juce::ChangeBroadcaster *source) {
  pgmInput.setText(
      audioProcessor.currentProgram < 65
          ? juce::String(defaultPatches[audioProcessor.currentProgram][1])
          : juce::String("unknown program"));
}
