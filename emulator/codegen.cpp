#include "codegen.h"

std::string getCodeLine(const LspInstr &inst,
                        std::vector<std::string> &initLines, bool usePipeline) {
  std::string codeLine = "";

  codeLine += "// ";
  codeLine += std::format("{:03x}", inst.pc + 0x80) + ": " +
              std::format("{:02x}", inst.ii) + " " +
              std::format("{:02x}", inst.rr) + " " +
              std::format("{:02x}", (uint8_t)inst.cc);
  codeLine += "\n";

  std::string oldAccAName = "readForStoreAccA()";
  std::string oldAccBName = "readForStoreAccB()";

  if (!usePipeline) {
    oldAccAName = "accA_" + std::to_string(inst.pc - pipelineWriteDelay);
    oldAccBName = "accB_" + std::to_string(inst.pc - pipelineWriteDelay);
  }

  if (inst.jumpDest) {
    codeLine += "jmp_loc_" + std::to_string(inst.pc) + ":\n";
  }

  if (inst.eramRead) {
    std::string addr =
        "eramPos + " + std::to_string((int32_t)inst.eramBaseAddr);
    if (inst.eramUseSecondTap) {
      addr += " + eramSecondTapOffs";
    }
    addr = "(" + addr + ") & 0xffff";
    codeLine += "eramReadValue = eram[" + addr + "] << 4;\n";
  }

  if ((inst.isMac || inst.isMul) && inst.writeCtrl != 0) {
    if (inst.writeCtrl == 0x08) {
      codeLine += "writeMemOffs(" + std::to_string(inst.memOffs) +
                  ", clamp_24(" + oldAccAName + "));\n";
    } else if (inst.writeCtrl == 0x10) {
      codeLine += "writeMemOffs(" + std::to_string(inst.memOffs) +
                  ", clamp_24(" + oldAccBName + "));\n";
    } else if (inst.writeCtrl == 0x18) {
      codeLine += "writeMemOffs(" + std::to_string(inst.memOffs) +
                  ", sign_extend_24(" + oldAccAName + "));\n";
    }
  }

  if (inst.isMac) {
    std::string accName = inst.macUseAccB ? "accB" : "accA";

    if (inst.cc != 0 || inst.macReplaceAcc) {
      if (inst.macReplaceAcc) {
        codeLine += accName + " = ";
      } else {
        codeLine += accName + " += ";
      }

      if (inst.useImmediate) {
        codeLine += std::to_string(inst.immediateValue);
      } else {
        if (inst.mulScaler == 5 && inst.cc == 0x20) {
          codeLine += "readMemOffs(" + std::to_string(inst.memOffs) + ")";
        } else {
          codeLine += "(readMemOffs(" + std::to_string(inst.memOffs) + ") * " +
                      std::to_string(inst.cc) + ") >> " +
                      std::to_string(inst.mulScaler);
        }
      }
      codeLine += ";";

      if (inst.macAbsValue) {
        codeLine +=
            "\nif (" + accName + " < 0) " + accName + " = -" + accName + ";";
      }
    }
  }

  if (inst.isMul) {
    std::string accName = inst.mulUseAccB ? "accB" : "accA";
    std::string coefName = inst.mulCoefSelect ? "multiplCoef2" : "multiplCoef1";

    std::string opA = "";
    std::string opB = "";
    std::string result = "";

    if (inst.mulLower) {
      opB = "((" + coefName + " & 0xffff) >> 9)";
    } else {
      opB = "(" + coefName + " >> 16)";
    }

    if (inst.useImmediate) {
      opA = std::to_string(inst.immediateValue);
    } else {
      opA = "readMemOffs(" + std::to_string(inst.memOffs) + ")";
    }

    if (inst.cc == 0x00) {
      result = "0";
    } else if (inst.mulLower) {
      result = "(" + opA + " * " + opB + ") >> " +
               std::to_string(inst.mulScaler + 7);
    } else {
      result =
          "(" + opA + " * " + opB + ") >> " + std::to_string(inst.mulScaler);
    }

    if (inst.mulNegate) {
      result = "-(" + result + ")";
    }

    if (!inst.mulReplaceAcc) {
      result = "(" + result + ") + clamp_24(" + accName + ")";
    }

    codeLine += accName + " = " + result + ";";
  }

  if (inst.isSpecial && inst.specialCase50d0) {
    std::string incr = "";

    if (inst.prevMem == 1 || inst.prevMem == 2 || inst.prevMem == 3 ||
        inst.prevMem == 4) {
      int prevShift = 0;
      if (inst.prevMem == 2)
        prevShift = 5;
      else if (inst.prevMem == 3)
        prevShift = 10;
      else if (inst.prevMem == 4)
        prevShift = 15;
      incr = std::to_string(((uint8_t)inst.cc) << prevShift);
    } else {
      incr = "((readMemOffs(" + std::to_string(inst.prevMem) + ") * " +
             std::to_string((uint8_t)inst.cc) + ") >> 7)";
    }

    incr += " >> " + std::to_string(inst.mulScaler + 1);

    std::string accName = inst.specialDestAccB ? "accB" : "accA";
    if (inst.specialReplaceAcc) {
      codeLine += accName + " = " + incr + ";";
    } else {
      codeLine += accName + " += " + incr + ";";
    }
  }

  if (inst.isSpecial && !inst.specialCase50d0) {
    std::string src = "";
    if (inst.writeCtrl == 0x08) {
      src = "clamp_24(" + oldAccAName + ")";
    } else if (inst.writeCtrl == 0x10) {
      src = "clamp_24(" + oldAccBName + ")";
    } else if (inst.writeCtrl == 0x18) {
      src = "sign_extend_24(" + oldAccAName + ")";
    }

    bool updatesAcc = false;

    if (inst.specialSlot == 0x0d) { // jmp if <0
      codeLine += "shouldJump = " + src + " < 0;";
    } else if (inst.specialSlot == 0x0e) { // jmp if >=0
      codeLine += "shouldJump = " + src + " >= 0;";
    } else if (inst.specialSlot == 0x0f) { // jmp
      codeLine += "shouldJump = true;";
    } else if (inst.specialSlot == 0x10) { // eram write latch
      codeLine += "eramWriteLatch = " + src + ";";
    } else if (inst.specialSlot == 0x13 &&
               inst.writeCtrl == 0x18) { // eram second tap offset
      codeLine += "eramSecondTapOffs = " + oldAccAName + ";\n";
      codeLine += "multiplCoef1 = (eramSecondTapOffs & 0x3ff) << 13;\n";
      codeLine += "eramSecondTapOffs >>= 10;";
    } else if (inst.specialSlot == 0x14) { // multiplCoef1
      codeLine += "multiplCoef1 = " + src + ";";
    } else if (inst.specialSlot == 0x15) { // multiplCoef2
      codeLine += "multiplCoef2 = " + src + ";";
    } else if (inst.specialSlot == 0x18) { // audio out
      std::string audioOut = inst.pc < (384 / 2) ? "audioOutR" : "audioOutL";
      codeLine += audioOut + " = " + src + ";";
      codeLine += "\nwriteMemOffs(0x78, " + audioOut + ");";
      updatesAcc = true;
    } else if (inst.specialSlot >= 0x1a && inst.specialSlot <= 0x1d) {
      src = "eramReadValue";
      codeLine += "writeMemOffs(" + std::to_string(inst.specialSlot | 0x70) +
                  ", " + src + ");";
      updatesAcc = true;
    } else if (inst.specialSlot == 0x1e) { // audio in
      std::string audioIn = inst.pc < (384 / 2) ? "audioInR" : "audioInL";
      src = audioIn;
      codeLine += "writeMemOffs(0x7e, " + src + ");";
      updatesAcc = true;
    } else {
      printf("ERROR: Unknown special write %02xx\n", inst.specialSlot);
    }

    if (updatesAcc) {
      codeLine += "\n";
      src += " * " + std::to_string(inst.cc);
      src = "(" + src + ") >> " + std::to_string(inst.mulScaler);
      std::string accName = inst.specialDestAccB ? "accB" : "accA";
      if (inst.specialReplaceAcc) {
        codeLine += accName + " = " + src + ";";
      } else {
        codeLine += accName + " += " + src + ";";
      }
    }
  }

  if (inst.eramWrite) {
    std::string addr =
        "eramPos + " + std::to_string((int32_t)inst.eramBaseAddr);
    addr = "(" + addr + ") & 0xffff";
    codeLine += "\neram[" + addr + "] = eramWriteLatch >> 4;";
  }

  if (!usePipeline) {
    if (inst.shouldStoreAccA) {
      initLines.push_back("int32_t accA_" + std::to_string(inst.pc) + ";");
      codeLine += "accA_" + std::to_string(inst.pc) + " = accA;\n";
    }
    if (inst.shouldStoreAccB) {
      initLines.push_back("int32_t accB_" + std::to_string(inst.pc) + ";");
      codeLine += "accB_" + std::to_string(inst.pc) + " = accB;\n";
    }
  } else {
    codeLine += "advanceAccA();advanceAccB();\n";
  }

  if (inst.jmp) {
    codeLine += "if (shouldJump) { shouldJump = false; goto jmp_loc_" +
                std::to_string(inst.jmpDest) + "; }\n";
  }

  return codeLine;
}

std::string convertToCode(const LspInstr instrCache[384]) {
  bool usePipeline = false;
  for (size_t pc = 0; pc < 384; pc++) {
    const LspInstr &instr = instrCache[pc];
    if (instr.jmp) {
      usePipeline = true;
      break;
    }
  }

  std::vector<std::string> initLines;
  std::vector<std::string> codeLines;
  for (size_t pc = 0; pc < 384; pc++) {
    const LspInstr &instr = instrCache[pc];
    codeLines.push_back(getCodeLine(instr, initLines, usePipeline));
  }

  std::string code = "";

  code += "#include \"runtime.h\"\n";
  code += "void Runtime::runCompiled() {\n";
  for (auto &line : initLines) {
    code += line;
    code += "\n";
  }
  code += "\n";
  for (auto &line : codeLines) {
    code += line;
    code += "\n";
  }
  code += "bufferPos = (bufferPos - 1) & 0x7f;\n";
  code += "eramPos -= 1;\n";
  code += "}\n";

  return code;
}
