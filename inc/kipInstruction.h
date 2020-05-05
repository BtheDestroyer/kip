#pragma once

#include <string>
#include <vector>
#include "kipUniversal.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace kip
{
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
    std::string command;
    std::vector<Argument> arguments;
  };

  class DLLMODE InterpretResult
  {
  public:
    InterpretResult(bool success, std::string str);
    operator bool() const;

    const bool success;
    const std::string str;
  };

  DLLMODE InterpretResult InterpretLine(std::string line);
  DLLMODE std::vector<InterpretResult> InterpretLines(std::vector<std::string> lines);
  DLLMODE Instruction ParseLine(std::string line);
  DLLMODE InterpretResult InterpretInstruction(Instruction inst);
  DLLMODE std::vector<InterpretResult> InterpretInstructions(std::vector<Instruction> inst);
}

#pragma warning(pop)
