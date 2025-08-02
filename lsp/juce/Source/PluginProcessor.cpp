/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const char *defaultPatches[65][2] = {
  {"Thru         ", "000000000000000000000000000000000000000000007f"},
  {"Stereo-EQ    ", "010000014501344800483800380000000000000000007f"},
  {"Spectrum     ", "0101003c414346423f3c3b02000000000000000000407f"},
  {"Enhancer     ", "010200407f00000000000000000000000000004340007f"},
  {"Humanizer    ", "0103003001007f0000000000000000000000004040407f"},
  {"Overdrive    ", "0110003001010000000000000000000000000040404060"},
  {"Distortion   ", "0111004c03010000000000000000000000000040384054"},
  {"Phaser       ", "012000241040107f000000000000000000000040400068"},
  {"Auto Wah     ", "0121000100443e28480100000000000000000040404060"},
  {"Rotary       ", "0122000671187f1178584060000000000000004040007f"},
  {"StereoFlanger", "0123000000100b18685a00000000000000004040400068"},
  {"Step Flanger ", "0124000a055f4f5a360000000000000000004040400060"},
  {"Tremolo      ", "012500013c60000000000000000000000000004040007f"},
  {"Auto Pan     ", "012600013c60000000000000000000000000004040007f"},
  {"Compressor   ", "0130004864000000000000000000000000000040404068"},
  {"Limiter      ", "013100550310000000000000000000000000004040407f"},
  {"Hexa Chorus  ", "01400018087f0542100000000000000000004040400070"},
  {"TremoloChorus", "014100100828283c600000000000000000007f4040007f"},
  {"Stereo Chorus", "01420000000a086f005a00000000000000004040400068"},
  {"Space D      ", "01430020087f5a00000000000000000000004040400060"},
  {"3D Chorus    ", "0144000a08480000000000000000000000004040400050"},
  {"Stereo Delay ", "015000657458010000007f00000000000000304040007f"},
  {"Mod Delay    ", "0151005a6c58010c155a7f00000000000000284040007f"},
  {"3 Tap Delay  ", "015200140007507f7f7f7f00000000000000304040007f"},
  {"4 Tap Delay  ", "0153003c1428007f7f7f7f507f0000000000304040007f"},
  {"Tm Ctrl Delay", "0154003c50507f4000000000000000000000304040007f"},
  {"Reverb       ", "015500037078680000000000000000000000404040007f"},
  {"Gate Reverb  ", "01560000050c0000000000000000000000002a403d0070"},
  {"3D Delay     ", "015700746a6e502840407f00000000000000304040007f"},
  {"2PitchShifter", "016000473e007f3b42000002400000000000304040005f"},
  {"Fb P.Shifter ", "0161004740545f024000000000000000000040403a007f"},
  {"3D Auto      ", "017000401901010000000000000000000000000000007f"},
  {"3D Manual    ", "017100400000000000000000000000000000000000007f"},
  {"Lo-Fi 1      ", "0172000105010000000000000000000000007f4040407f"},
  {"Lo-Fi 2      ", "0173000101240040017f00007f00007f00017f4040407f"},
  {"OD -> Chorus ", "02000030400100000a0848004000000000000040400050"},
  {"OD -> Flanger", "0201003040010000100b28682000000000000040400050"},
  {"OD -> Delay  ", "02020030400100006f507f003000000000000040400050"},
  {"DS -> Chorus ", "02030030400100000a0848004000000000000040400048"},
  {"DS -> Flanger", "02040030400100000b0b18682000000000000040400048"},
  {"DS -> Delay  ", "02050030400100006f507f003000000000000040400048"},
  {"EH -> Chorus ", "020600407f000000400865004000000000000040400050"},
  {"EH -> Flanger", "020700407f000000100b18683000000000000040400060"},
  {"EH -> Delay  ", "020800407f0000006f507f003000000000000040400058"},
  {"Cho -> Delay ", "0209000a097800406f507f00300000000000004040007f"},
  {"FL -> Delay  ", "020a00100b1868406f507f00300000000000004040007f"},
  {"Cho-> Flanger", "020b000a08780040100b18684000000000000040400070"},
  {"Rotary Multi ", "0300000d0140480040400671187f117858406000000060"},
  {"GTR Multi 1  ", "0400006450640100500001454a0100081e66283c220f6e"},
  {"GTR Multi 2  ", "040100467f5a0100500201014c38024536000860660050"},
  {"GTR Multi 3  ", "040200013c0a0101500201404001000800593220401e58"},
  {"CleanGtMulti1", "040300327f4b014c000245340008284f6418287f1e005f"},
  {"CleanGtMulti2", "0404000137281d50014c480040400008146664060f504c"},
  {"Bass Multi   ", "04050048644b010030000001424801444000051466404c"},
  {"Rhodes Multi ", "04060040402410201040007f0a0840687f01023c40017f"},
  {"KeyboardMulti", "0500003214430002453d4740002724085a504b14402860"},
  {"Cho _ Delay  ", "1100000a087800406f507f00280000000000007f7f7f60"},
  {"FL _ Delay   ", "110100100b1868406f507f00300000000000007f7f7f60"},
  {"Cho _ Flanger", "1102001008780040100b1868400000000000007f7f7f58"},
  {"OD1 _ OD2    ", "1103000030010100014c030100000000000000607f547f"},
  {"OD _ Rotary  ", "11040000300101000671187f11785840600000607f7f7f"},
  {"OD _ Phaser  ", "1105000030010100241040107f000000000000607f7f7f"},
  {"OD _ AutoWah ", "11060000300101000100443e28480100000000607f7f7f"},
  {"PH _ Rotary  ", "110700241040107f0671187f117858406000007f7f7f7f"},
  {"PH _ AutoWah ", "110800241040107f0100443e284801000000007f7f7f7f"},
};

//==============================================================================
LSP_juceAudioProcessor::LSP_juceAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
  lsp = new LspState();
  mcu = new MCU();

  mcu->ProgGen_Prepare((const uint8_t *)BinaryData::sc88pro_valid_bin,
                       BinaryData::sc88pro_valid_binSize);
}

LSP_juceAudioProcessor::~LSP_juceAudioProcessor() {
  lspMutex.enter();
  if (lsp != nullptr) {
    delete lsp;
    lsp = nullptr;
  }
  if (mcu != nullptr) {
    delete mcu;
    mcu = nullptr;
  }
  lspMutex.exit();
}

//==============================================================================
const juce::String LSP_juceAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool LSP_juceAudioProcessor::acceptsMidi() const { return false; }

bool LSP_juceAudioProcessor::producesMidi() const { return false; }

bool LSP_juceAudioProcessor::isMidiEffect() const { return false; }

double LSP_juceAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int LSP_juceAudioProcessor::getNumPrograms() { return 65; }

int LSP_juceAudioProcessor::getCurrentProgram() { return currentProgram; }

void LSP_juceAudioProcessor::setCurrentProgram(int index) {
  auto str = juce::String(defaultPatches[index][1]);
  uint8_t all_params[23] = {0};
  for (int i = 0; i < str.length() && i < 23 * 2; i += 2) {
    if (i + 1 < str.length()) {
      all_params[i / 2] = (uint8_t)str.substring(i, i + 2).getHexValue32();
    }
  }
  regenProgram(all_params[0], all_params[1], &all_params[2]);

  sendChangeMessage();
}

const juce::String LSP_juceAudioProcessor::getProgramName(int index) {
  return juce::String(defaultPatches[index][0]);
}

void LSP_juceAudioProcessor::changeProgramName(int index,
                                               const juce::String &newName) {}

//==============================================================================
void LSP_juceAudioProcessor::prepareToPlay(double sampleRate,
                                           int samplesPerBlock) {
  double ratio = sampleRate / 32000.0;

  dspBuffer.setSize(2, (int)std::ceil(samplesPerBlock * ratio));

  if (resampleL)
    resample_close(resampleL);
  if (resampleR)
    resample_close(resampleR);
  resampleL = resample_open(1, ratio, ratio);
  resampleR = resample_open(1, ratio, ratio);
}

void LSP_juceAudioProcessor::regenProgram(uint8_t group, uint8_t type,
                                          uint8_t params[21]) {
  mcu->ProgGen_Generate(group, type, params,
                        (const uint8_t *)BinaryData::state_loadlsp_bin);
  lspMutex.enter();
  memcpy(lsp->iram, mcu->lsp_temp, sizeof(mcu->lsp_temp));
  lsp->iram[0x081] |= 0x7f0000;
  lsp->iram[0x1f3] |= 0x7f0000;
  lsp->parseProgram(true);
  lspMutex.exit();
}

void LSP_juceAudioProcessor::releaseResources() {}

bool LSP_juceAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  return true;
}

void LSP_juceAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                          juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;

  const float *inL = buffer.getReadPointer(0);
  const float *inR = buffer.getReadPointer(1);
  float *bufL = dspBuffer.getWritePointer(0);
  float *bufR = dspBuffer.getWritePointer(1);
  float *outL = buffer.getWritePointer(0);
  float *outR = buffer.getWritePointer(1);

  int destSampleRate = getSampleRate();
  int nFrames = buffer.getNumSamples();

  // double renderBufferFramesFloat = (double)nFrames / destSampleRate * 32000;
  // unsigned int renderBufferFrames = std::ceil(renderBufferFramesFloat);
  // double currentError = renderBufferFrames - renderBufferFramesFloat;

  // int limit = nFrames / 4;
  // if (samplesError > limit) {
  //   // printf("compensating neg %d\n", limit);
  //   renderBufferFrames -= limit;
  //   currentError -= limit;
  // } else if (-samplesError > limit) {
  //   // printf("compensating pos %d\n", limit);
  //   renderBufferFrames += limit;
  //   currentError += limit;
  // }

  int renderBufferFrames = nFrames;

  lspMutex.enter();
  for (int i = 0; i < renderBufferFrames; ++i) {
    // double pos = (double)i / std::ceil(renderBufferFramesFloat) * nFrames;
    // int posMin = (int)std::floor(pos);
    // int posMax = (int)std::ceil(pos);
    // double pMinL = inL[posMin];
    // double pMaxL = inL[posMax];
    // if (posMax >= nFrames) {
    //   pMaxL = pMinL;
    // }
    // double pMinR = inR[posMin];
    // double pMaxR = inR[posMax];
    // if (posMax >= nFrames) {
    //   pMaxL = pMinR;
    // }

    // double interpL = pMinL + (pMaxL - pMinL) * (pos - posMin);
    // double interpR = pMinR + (pMaxR - pMinR) * (pos - posMin);
    
    double interpL = inL[i];
    double interpR = inL[i];

    lsp->audioInL = interpL * 0x7fff;
    lsp->audioInR = interpR * 0x7fff;

    lsp->audioInL <<= 6;
    lsp->audioInR <<= 6;

    lsp->runProgram();

    lsp->audioOutL >>= 6;
    lsp->audioOutR >>= 6;

    bufL[i] = lsp->audioOutL / ((float)0x7fff);
    bufR[i] = lsp->audioOutR / ((float)0x7fff);

    outL[i] = bufL[i];
    outR[i] = bufR[i];
  }
  lspMutex.exit();

  // double ratio = (double)destSampleRate / 32000;

  // int inUsedL = 0;
  // int inUsedR = 0;
  // int resOutL = 0;
  // int resOutR = 0;

  // resOutL = resample_process(resampleL, ratio, bufL, renderBufferFrames, false,
  //                            &inUsedL, outL, nFrames);
  // resOutR = resample_process(resampleR, ratio, bufR, renderBufferFrames, false,
  //                            &inUsedR, outR, nFrames);

  // samplesError += currentError;
  // // printf("error: %f total: %f\n", currentError, samplesError);

  // if (inUsedL == 0 || inUsedR == 0) {
  //   samplesError = 0;
  //   printf("click: %d %d\n", resOutL, resOutR);
  // }
}

//==============================================================================
bool LSP_juceAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor *LSP_juceAudioProcessor::createEditor() {
  return new LSP_juceAudioProcessorEditor(*this);
}

//==============================================================================
void LSP_juceAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {}

void LSP_juceAudioProcessor::setStateInformation(const void *data,
                                                 int sizeInBytes) {}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new LSP_juceAudioProcessor();
}
