#pragma once

#include <string>
#include <vector>

// Reads a 16-bit PCM mono or stereo WAV file into a vector of int16_t.
// Returns true on success, false on failure.
bool read_wav(const std::string &filename, std::vector<int16_t> &samples,
              int &sampleRate, int &numChannels);

// Writes a 16-bit PCM mono or stereo WAV file from a vector of int16_t.
// Returns true on success, false on failure.
bool write_wav(const std::string &filename, const std::vector<int16_t> &samples,
               int sampleRate, int numChannels);
