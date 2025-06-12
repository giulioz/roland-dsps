#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "emulator.h"

std::string getCodeLine(const LspInstr &inst,
                        std::vector<std::string> &initLines, bool usePipeline);

std::string convertToCode(const LspInstr instrCache[384]);
