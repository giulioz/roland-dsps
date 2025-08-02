#include "wavutils.h"
#include <cstdint>
#include <fstream>

// Reads a 16-bit PCM mono or stereo WAV file into a vector of int16_t.
// Returns true on success, false on failure.
bool read_wav(const std::string &filename, std::vector<int16_t> &samples,
              int &sampleRate, int &numChannels) {
  samples.clear();
  std::ifstream file(filename, std::ios::binary);
  if (!file)
    return false;

  char riff[4];
  file.read(riff, 4);
  if (std::string(riff, 4) != "RIFF")
    return false;
  file.ignore(4); // file size
  char wave[4];
  file.read(wave, 4);
  if (std::string(wave, 4) != "WAVE")
    return false;

  // Read chunks
  bool fmtFound = false, dataFound = false;
  int bitsPerSample = 0;
  uint32_t dataSize = 0;
  while (file && (!fmtFound || !dataFound)) {
    char chunkId[4];
    file.read(chunkId, 4);
    uint32_t chunkSize = 0;
    file.read(reinterpret_cast<char *>(&chunkSize), 4);

    if (std::string(chunkId, 4) == "fmt ") {
      uint16_t audioFormat = 0;
      file.read(reinterpret_cast<char *>(&audioFormat), 2);
      file.read(reinterpret_cast<char *>(&numChannels), 2);
      file.read(reinterpret_cast<char *>(&sampleRate), 4);
      file.ignore(6); // byte rate + block align
      file.read(reinterpret_cast<char *>(&bitsPerSample), 2);
      file.ignore(chunkSize - 16);
      if (audioFormat != 1 || bitsPerSample != 16)
        return false;
      fmtFound = true;
    } else if (std::string(chunkId, 4) == "data") {
      dataSize = chunkSize;
      samples.resize(dataSize / 2);
      file.read(reinterpret_cast<char *>(samples.data()), dataSize);
      dataFound = true;
    } else {
      file.ignore(chunkSize);
    }
  }
  return fmtFound && dataFound;
}

// Writes a 16-bit PCM mono or stereo WAV file from a vector of int16_t.
// Returns true on success, false on failure.
bool write_wav(const std::string &filename, const std::vector<int16_t> &samples,
               int sampleRate, int numChannels) {
  std::ofstream file(filename, std::ios::binary);
  if (!file)
    return false;

  uint32_t dataChunkSize = samples.size() * sizeof(int16_t);
  uint32_t fmtChunkSize = 16;
  uint32_t riffChunkSize = 4 + (8 + fmtChunkSize) + (8 + dataChunkSize);

  // RIFF header
  file.write("RIFF", 4);
  file.write(reinterpret_cast<const char *>(&riffChunkSize), 4);
  file.write("WAVE", 4);

  // fmt chunk
  file.write("fmt ", 4);
  file.write(reinterpret_cast<const char *>(&fmtChunkSize), 4);
  uint16_t audioFormat = 1;
  file.write(reinterpret_cast<const char *>(&audioFormat), 2);
  file.write(reinterpret_cast<const char *>(&numChannels), 2);
  file.write(reinterpret_cast<const char *>(&sampleRate), 4);
  uint32_t byteRate = sampleRate * numChannels * 2;
  uint16_t blockAlign = numChannels * 2;
  uint16_t bitsPerSample = 16;
  file.write(reinterpret_cast<const char *>(&byteRate), 4);
  file.write(reinterpret_cast<const char *>(&blockAlign), 2);
  file.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

  // data chunk
  file.write("data", 4);
  file.write(reinterpret_cast<const char *>(&dataChunkSize), 4);
  file.write(reinterpret_cast<const char *>(samples.data()), dataChunkSize);

  return true;
}
