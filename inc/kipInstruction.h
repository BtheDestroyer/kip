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
    Argument(const std::string& string);

    uint32_t GetAddr() const;
    uint8_t GetByte() const;
    const std::string GetString() const;

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
      uint32_t line = 0;
    };

    Instruction();
    Instruction(std::string line);
    Instruction(std::string line, Context& context);

    // Instructions

    // Storage
    InterpretResult STB(Context* context) const;
    InterpretResult STA(Context* context) const;
    InterpretResult STS(Context* context) const;
    InterpretResult FIL(Context* context) const;
    InterpretResult CPY(Context* context) const;
    InterpretResult BIN(Context* context) const;
    InterpretResult SAV(Context* context) const;

    // Debug reads
    InterpretResult RDB(Context* context) const;
    InterpretResult RDA(Context* context) const;
    InterpretResult RDS(Context* context) const;

    // Jumps
    InterpretResult JMP(Context* context) const;
    InterpretResult JEQ(Context* context) const;
    InterpretResult JNE(Context* context) const;
    InterpretResult JGT(Context* context) const;
    InterpretResult JLT(Context* context) const;
    InterpretResult JGE(Context* context) const;
    InterpretResult JLE(Context* context) const;

    // Arithmetic
    InterpretResult ADB(Context* context) const;
    InterpretResult ADA(Context* context) const;
    InterpretResult SBB(Context* context) const;
    InterpretResult SBA(Context* context) const;
    InterpretResult MLB(Context* context) const;
    InterpretResult MLA(Context* context) const;
    InterpretResult DVB(Context* context) const;
    InterpretResult DVA(Context* context) const;
    InterpretResult MDB(Context* context) const;
    InterpretResult MDA(Context* context) const;

    // Increment/decrement
    InterpretResult INB(Context* context) const;
    InterpretResult INA(Context* context) const;
    InterpretResult DCB(Context* context) const;
    InterpretResult DCA(Context* context) const;
    
    // Bit manipulation
    InterpretResult BLS(Context* context) const;
    InterpretResult BRS(Context* context) const;
    InterpretResult ROL(Context* context) const;
    InterpretResult ROR(Context* context) const;
    InterpretResult AND(Context* context) const;
    InterpretResult BOR(Context* context) const;
    InterpretResult XOR(Context* context) const;
    InterpretResult NOT(Context* context) const;
    
    // Control flow
    InterpretResult HLT(Context* context) const;
    InterpretResult PUB(Context* context) const;
    InterpretResult PUA(Context* context) const;
    InterpretResult PUS(Context* context) const;
    InterpretResult POB(Context* context) const;
    InterpretResult POA(Context* context) const;
    InterpretResult POS(Context* context) const;
    InterpretResult CAL(Context* context) const;

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
  DLLMODE std::vector<Instruction> BuildInstructions(Instruction::Context& context, std::vector<std::string>& lines);
  DLLMODE std::vector<InterpretResult> InterpretLines(std::vector<std::string> &lines, uint8_t verbosity = 255);
  DLLMODE std::vector<InterpretResult> InterpretLines(std::vector<std::string> &lines, std::string folder, uint8_t verbosity = 255);
  DLLMODE std::vector<InterpretResult> InterpretInstructions(const std::vector<Instruction> &inst, uint8_t verbosity = 255);
  DLLMODE std::vector<InterpretResult> InterpretInstructions(const std::vector<Instruction> &inst, Instruction::Context &context, uint8_t verbosity = 255);
}

#pragma warning(pop)
