#pragma once

#include <string>
#include <map>
#include <vector>
#include "kipUniversal.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace kip
{
  class DLLMODE InterpretResult
  {
  public:
    InterpretResult(bool success, std::string str);
    operator bool() const;

    const bool success;
    const std::string str;
  };

  class DLLMODE Argument
  {
  public:
    uint32_t GetAddr();
    uint8_t GetByte();

    uint32_t data = 0;
    uint8_t dereferenceCount = 0;
  };

  class DLLMODE Instruction
  {
  public:
    // Map of label to line
    struct DLLMODE Context
    {
      std::map<std::string, uint32_t> labels;
      std::string folder;
    };

    Instruction(std::string line);
    Instruction(std::string line, Context context);

    InterpretResult STB(uint32_t* line);
    InterpretResult STA(uint32_t* line);
    InterpretResult RDB(uint32_t* line);
    InterpretResult RDA(uint32_t* line);
    InterpretResult FIL(uint32_t* line);
    InterpretResult ADB(uint32_t* line);
    InterpretResult ADA(uint32_t* line);
    InterpretResult SBB(uint32_t* line);
    InterpretResult SBA(uint32_t* line);
    InterpretResult JMP(uint32_t* line);
    InterpretResult JEQ(uint32_t* line);
    InterpretResult JNE(uint32_t* line);
    InterpretResult JGT(uint32_t* line);
    InterpretResult JLT(uint32_t* line);
    InterpretResult JGE(uint32_t* line);
    InterpretResult JLE(uint32_t* line);
    InterpretResult MLB(uint32_t* line);
    InterpretResult MLA(uint32_t* line);
    InterpretResult DVB(uint32_t* line);
    InterpretResult DVA(uint32_t* line);
    InterpretResult MDB(uint32_t* line);
    InterpretResult MDA(uint32_t* line);
    InterpretResult BLS(uint32_t* line);
    InterpretResult BRS(uint32_t* line);
    InterpretResult ROL(uint32_t* line);
    InterpretResult ROR(uint32_t* line);
    InterpretResult AND(uint32_t* line);
    InterpretResult BOR(uint32_t* line);
    InterpretResult XOR(uint32_t* line);
    InterpretResult NOT(uint32_t* line);
    InterpretResult HLT(uint32_t* line);
    InterpretResult INB(uint32_t* line);
    InterpretResult INA(uint32_t* line);
    InterpretResult DCB(uint32_t* line);
    InterpretResult DCA(uint32_t* line);
    InterpretResult PUB(uint32_t* line);
    InterpretResult PUA(uint32_t* line);
    InterpretResult POB(uint32_t* line);
    InterpretResult POA(uint32_t* line);
    InterpretResult CPY(uint32_t* line);
    InterpretResult CAL(uint32_t* line);

    uint8_t id;
    std::vector<Argument> arguments;
  };

  DLLMODE std::string RemoveComments(std::string line);
  DLLMODE InterpretResult InterpretLine(std::string line);
  DLLMODE InterpretResult LoadFile(std::string filename, std::vector<std::string>& lines);
  DLLMODE std::vector<InterpretResult> BuildContext(Instruction::Context& context, std::vector<std::string>& lines);
  DLLMODE std::vector<InterpretResult> BuildContextImports(Instruction::Context& context, std::vector<std::string>& lines);
  DLLMODE std::vector<InterpretResult> BuildContextLabels(Instruction::Context& context, std::vector<std::string>& lines);
  DLLMODE std::vector<InterpretResult> InterpretLines(std::vector<std::string> lines);
  DLLMODE std::vector<InterpretResult> InterpretLines(std::vector<std::string> lines, std::string folder);
  DLLMODE std::vector<InterpretResult> InterpretInstructions(std::vector<Instruction> inst);
  DLLMODE std::vector<InterpretResult> InterpretInstructions(std::vector<Instruction> inst, Instruction::Context context);
}

#pragma warning(pop)
