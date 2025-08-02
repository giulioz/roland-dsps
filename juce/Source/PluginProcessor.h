/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../../emulator/emulator.h"
#include "../../sc88_pgmgen/mcu.h"
#include "resample/libresample.h"


extern const char *defaultPatches[65][2];

//==============================================================================
/**
 */
class LSP_juceAudioProcessor : public juce::AudioProcessor,
                               public juce::ChangeBroadcaster {
public:
  //==============================================================================
  LSP_juceAudioProcessor();
  ~LSP_juceAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
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
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  //==============================================================================

  LspState *lsp = nullptr;
  juce::SpinLock lspMutex;

  void *resampleL = 0;
  void *resampleR = 0;
  int savedDestSampleRate = 0;
  double samplesError = 0;

  juce::AudioBuffer<float> dspBuffer;

  MCU *mcu;
  void regenProgram(uint8_t group, uint8_t type, uint8_t params[21]);

  int currentProgram = 0;

private:
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LSP_juceAudioProcessor)
};
