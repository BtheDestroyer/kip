#pragma once

#include <string>
#include <map>
#include <vector>
#include "kipUniversal.h"

#define KIP_VERBOSITY_RESERVE_SMALL uint8_t(100)
#define KIP_VERBOSITY_RESERVE_LARGE uint8_t(200)

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
    Argument();
    Argument(uint32_t data, uint8_t dereferenceCount = 0);
    Argument(std::string stringLabel);

    uint32_t GetAddr();
    uint8_t GetByte();

    uint32_t data = 0;
    uint8_t dereferenceCount = 0;
    std::string stringLabel = "";
    enum class Type {
      INVALID,
      DATA,
      STRING,
      COUNT
    } type;
  };

  class DLLMODE Instruction
  {
  public:
    // Map of label to line
    struct DLLMODE Context
    {
      std::map<std::string, Argument> labels;
      std::string folder;
    };

    Instruction();
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

    const std::string line;
    uint8_t id;
    std::vector<Argument> arguments;
  };

  DLLMODE std::string RemoveComments(std::string line);
  DLLMODE InterpretResult InterpretLine(std::string line);
  DLLMODE InterpretResult LoadFile(std::string filename, std::vector<std::string>& lines);
  DLLMODE std::vector<InterpretResult> BuildContext(Instruction::Context& context, std::vector<std::string>& lines);
  DLLMODE std::vector<InterpretResult> BuildContextImports(Instruction::Context& context, std::vector<std::string>& lines);
  DLLMODE std::vector<InterpretResult> BuildContextLabels(Instruction::Context& context, std::vector<std::string>& lines);
  DLLMODE std::vector<InterpretResult> InterpretLines(std::vector<std::string> &lines, uint8_t verbosity = 255);
  DLLMODE std::vector<InterpretResult> InterpretLines(std::vector<std::string> &lines, std::string folder, uint8_t verbosity = 255);
  DLLMODE std::vector<InterpretResult> InterpretInstructions(std::vector<Instruction> &inst, uint8_t verbosity = 255);
  DLLMODE std::vector<InterpretResult> InterpretInstructions(std::vector<Instruction> &inst, Instruction::Context &context, uint8_t verbosity = 255);
}

#pragma warning(pop)
