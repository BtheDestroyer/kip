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
    const std::string& GetString() const;

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

    // Instructions

    // Storage
    InterpretResult STB(uint32_t* line) const;
    InterpretResult STA(uint32_t* line) const;
    InterpretResult STS(uint32_t* line) const;
    InterpretResult FIL(uint32_t* line) const;
    InterpretResult CPY(uint32_t* line) const;
    InterpretResult BIN(uint32_t* line) const;
    InterpretResult SAV(uint32_t* line) const;

    // Debug reads
    InterpretResult RDB(uint32_t* line) const;
    InterpretResult RDA(uint32_t* line) const;
    InterpretResult RDS(uint32_t* line) const;

    // Jumps
    InterpretResult JMP(uint32_t* line) const;
    InterpretResult JEQ(uint32_t* line) const;
    InterpretResult JNE(uint32_t* line) const;
    InterpretResult JGT(uint32_t* line) const;
    InterpretResult JLT(uint32_t* line) const;
    InterpretResult JGE(uint32_t* line) const;
    InterpretResult JLE(uint32_t* line) const;

    // Arithmetic
    InterpretResult ADB(uint32_t* line) const;
    InterpretResult ADA(uint32_t* line) const;
    InterpretResult SBB(uint32_t* line) const;
    InterpretResult SBA(uint32_t* line) const;
    InterpretResult MLB(uint32_t* line) const;
    InterpretResult MLA(uint32_t* line) const;
    InterpretResult DVB(uint32_t* line) const;
    InterpretResult DVA(uint32_t* line) const;
    InterpretResult MDB(uint32_t* line) const;
    InterpretResult MDA(uint32_t* line) const;

    // Increment/decrement
    InterpretResult INB(uint32_t* line) const;
    InterpretResult INA(uint32_t* line) const;
    InterpretResult DCB(uint32_t* line) const;
    InterpretResult DCA(uint32_t* line) const;
    
    // Bit manipulation
    InterpretResult BLS(uint32_t* line) const;
    InterpretResult BRS(uint32_t* line) const;
    InterpretResult ROL(uint32_t* line) const;
    InterpretResult ROR(uint32_t* line) const;
    InterpretResult AND(uint32_t* line) const;
    InterpretResult BOR(uint32_t* line) const;
    InterpretResult XOR(uint32_t* line) const;
    InterpretResult NOT(uint32_t* line) const;
    
    // Control flow
    InterpretResult HLT(uint32_t* line) const;
    InterpretResult PUB(uint32_t* line) const;
    InterpretResult PUA(uint32_t* line) const;
    InterpretResult PUS(uint32_t* line) const;
    InterpretResult POB(uint32_t* line) const;
    InterpretResult POA(uint32_t* line) const;
    InterpretResult POS(uint32_t* line) const;
    InterpretResult CAL(uint32_t* line) const;

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
