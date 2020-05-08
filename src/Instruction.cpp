#include "pch.h"

#include <algorithm>
#include <string>
#include <vector>
#include <sstream>

#include "kipInstruction.h"
#include "kipMemory.h"

namespace kip
{
  const struct {
    const char* const string;
    const uint8_t argumentCount;
    InterpretResult (Instruction::*function)(uint32_t* line);
  } instructionTable[] = {
    { "???", 0, nullptr },
    { "STB", 2, &Instruction::STB },
    { "STA", 2, &Instruction::STA },
    { "RDB", 1, &Instruction::RDB },
    { "RDA", 1, &Instruction::RDA },
    { "FIL", 3, &Instruction::FIL },
    { "ADD", 3, &Instruction::ADD },
    { "SUB", 3, &Instruction::SUB },
    { "JMP", 1, &Instruction::JMP },
    { "JEQ", 3, &Instruction::JEQ },
    { "JNE", 3, &Instruction::JNE },
    { "JGT", 3, &Instruction::JGT },
    { "JLT", 3, &Instruction::JLT },
    { "MUL", 3, &Instruction::MUL },
    { "DIV", 3, &Instruction::DIV },
    { "MOD", 3, &Instruction::MOD },
    { "BLS", 3, &Instruction::BLS },
    { "BRS", 3, &Instruction::BRS },
    { "ROL", 3, &Instruction::ROL },
    { "ROR", 3, &Instruction::ROR },
    { "AND", 3, &Instruction::AND },
    { "BOR", 3, &Instruction::BOR },
    { "XOR", 3, &Instruction::XOR },
    { "NOT", 2, &Instruction::NOT },
    { "HLT", 0, &Instruction::HLT },
    { "INC", 1, &Instruction::INC },
    { "DEC", 1, &Instruction::DEC },
    { "PUB", 1, &Instruction::PUB },
    { "PUA", 1, &Instruction::PUA },
    { "POB", 1, &Instruction::POB },
    { "POA", 1, &Instruction::POA },
  };

  uint8_t GetInstructionIndex(std::string instruction)
  {
    for (uint8_t i = 1; i < sizeof(instructionTable) / sizeof(*instructionTable); ++i)
      if (instruction == instructionTable[i].string)
        return i;
    return 0;
  }

//////////////////////////////////////////////////////////////

  InterpretResult Instruction::STB(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint32_t B = arguments[1].GetAddr();
    if (WriteByte(B, A))
      return InterpretResult(true, std::to_string(int(B)) + "<=" + std::to_string(int(A)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::STA(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint32_t B = arguments[1].GetAddr();
    if (WriteBytes(B, (uint8_t*)(&A), 4))
      return InterpretResult(true, std::to_string(int(B)) + "<=" + std::to_string(int(A)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::RDB(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint8_t out;
    if (ReadByte(A, out))
      return InterpretResult(true, std::to_string(int(A)) + "=>" + std::to_string(int(out)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::RDA(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint32_t out;
    if (ReadBytes(A, (uint8_t*)(&out), 4))
      return InterpretResult(true, std::to_string(int(A)) + "=>" + std::to_string(int(out)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::FIL(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint32_t B = arguments[1].GetAddr();
    uint32_t C = arguments[2].GetAddr();
    for (uint32_t i = B; i < B + C; ++i)
      if (!WriteByte(i, A))
        return InterpretResult(false, "Address " + std::to_string(i) + " not mapped");
    return InterpretResult(true, "[" + std::to_string(int(B)) + ", " + std::to_string(int(B + C)) + ")<=" + std::to_string(int(A)));
  }

  InterpretResult Instruction::ADD(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A + B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A + B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::SUB(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A - B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A - B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::JMP(uint32_t* line)
  {
    if (!line)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    *line = arguments[0].GetAddr() - 1;
    return InterpretResult(true, "pc<=" + std::to_string(*line));
  }

  InterpretResult Instruction::JEQ(uint32_t* line)
  {
    if (!line)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    uint32_t A = arguments[0].GetAddr();
    uint8_t  B = arguments[1].GetByte();
    uint8_t  C = arguments[2].GetByte();
    if (B == C)
    {
      *line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(*line));
  }

  InterpretResult Instruction::JNE(uint32_t* line)
  {
    if (!line)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    uint32_t A = arguments[0].GetAddr();
    uint8_t  B = arguments[1].GetByte();
    uint8_t  C = arguments[2].GetByte();
    if (B != C)
    {
      *line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(*line));
  }

  InterpretResult Instruction::JGT(uint32_t* line)
  {
    if (!line)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    uint32_t A = arguments[0].GetAddr();
    uint8_t  B = arguments[1].GetByte();
    uint8_t  C = arguments[2].GetByte();
    if (B > C)
    {
      *line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(*line));
  }

  InterpretResult Instruction::JLT(uint32_t* line)
  {
    if (!line)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    uint32_t A = arguments[0].GetAddr();
    uint8_t  B = arguments[1].GetByte();
    uint8_t  C = arguments[2].GetByte();
    if (B < C)
    {
      *line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(*line));
  }

  InterpretResult Instruction::JGE(uint32_t* line)
  {
    if (!line)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    uint32_t A = arguments[0].GetAddr();
    uint8_t  B = arguments[1].GetByte();
    uint8_t  C = arguments[2].GetByte();
    if (B >= C)
    {
      *line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(*line));
  }

  InterpretResult Instruction::JLE(uint32_t* line)
  {
    if (!line)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    uint32_t A = arguments[0].GetAddr();
    uint8_t  B = arguments[1].GetByte();
    uint8_t  C = arguments[2].GetByte();
    if (B <= C)
    {
      *line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(*line));
  }

  InterpretResult Instruction::MUL(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A * B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A * B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::DIV(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A / B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A / B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::MOD(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A % B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A % B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::BLS(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A << B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A << B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::BRS(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A >> B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A >> B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::ROL(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    B %= 8;
    uint16_t r = uint16_t(A) << B;
    r = (r & 0x0F) | ((r & 0xF0) >> 8);
    if (WriteByte(C, uint8_t(r)))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(r)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::ROR(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    B %= 8;
    uint8_t r = 0;
    r |= (A >> B) & 0xF;
    r |= A << (8 - B);
    if (WriteByte(C, uint8_t(r)))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(r)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::AND(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, uint8_t(A & B)))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A & B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::BOR(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, uint8_t(A | B)))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A | B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::XOR(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, uint8_t(A ^ B)))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A ^ B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::NOT(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint32_t B = arguments[1].GetAddr();
    if (WriteByte(B, uint8_t(~A)))
      return InterpretResult(true, std::to_string(int(B)) + "<=" + std::to_string(int(~A)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::HLT(uint32_t* line)
  {
    if (!line)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    *line = uint32_t(-1);
    return InterpretResult(true, "Halted program");
  }

  InterpretResult Instruction::INC(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint8_t  v;
    if (ReadByte(A, v) && WriteByte(A, ++v))
      return InterpretResult(true, std::to_string(int(A)) + "<=" + std::to_string(int(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::DEC(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint8_t  v;
    if (ReadByte(A, v) && WriteByte(A, --v))
      return InterpretResult(true, std::to_string(int(A)) + "<=" + std::to_string(int(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::PUB(uint32_t* line)
  {
    uint32_t s = 0;
    uint8_t  A = arguments[0].GetByte();
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (WriteByte(s, A))
    {
      if (!SetStackPointer(s - 1))
        return InterpretResult(false, "Stack pointer is not mapped post-decrement (" + std::to_string(s - 1) + ")");
      return InterpretResult(true, std::to_string(int(s)) + "<=" + std::to_string(int(A)));
    }
    return InterpretResult(false, "Stack pointer is not mapped post-write (" + std::to_string(s) + ")");
  }

  InterpretResult Instruction::PUA(uint32_t* line)
  {
    uint32_t s = 0;
    uint32_t A = arguments[0].GetAddr();
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (WriteBytes(s - 3, (uint8_t*)(&A), 4))
    {
      if (!SetStackPointer(s - 4))
        return InterpretResult(false, "Stack pointer is not mapped post-decrement (" + std::to_string(s - 4) + ")");
      return InterpretResult(true, std::to_string(int(s - 3)) + "<=" + std::to_string(int(A)));
    }
    return InterpretResult(false, "Stack pointer is not mapped post-write (" + std::to_string(s - 3) + ")");
  }

  InterpretResult Instruction::POB(uint32_t* line)
  {
    uint32_t s = 0;
    uint32_t A = arguments[0].GetAddr();
    uint8_t  v = 0;
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (ReadByte(++s, v))
    {
      if (!WriteByte(A, v))
        return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
      if (!SetStackPointer(s))
        return InterpretResult(false, "Stack pointer is not mapped post-increment (" + std::to_string(s) + ")");
      return InterpretResult(true, std::to_string(int(A)) + "<=" + std::to_string(int(v)));
    }
    return InterpretResult(false, "Stack pointer is not mapped post-read (" + std::to_string(s) + ")");
  }

  InterpretResult Instruction::POA(uint32_t* line)
  {
    uint32_t s = 0;
    uint32_t A = arguments[0].GetAddr();
    uint32_t v = 0;
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (ReadBytes(++s, (uint8_t*)(&v), 4))
    {
      if (!WriteBytes(A, (uint8_t*)(&v), 4))
        return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
      if (!SetStackPointer(s + 3))
        return InterpretResult(false, "Stack pointer is not mapped post-increment (" + std::to_string(s + 3) + ")");
      return InterpretResult(true, std::to_string(int(A)) + "<=" + std::to_string(int(v)));
    }
    return InterpretResult(false, "Stack pointer is not mapped post-read (" + std::to_string(s) + ")");
  }


//////////////////////////////////////////////////////////////

  uint32_t Argument::GetAddr()
  {
    uint32_t d = data;
    for (uint8_t i = dereferenceCount; i > 0; --i)
      ReadBytes(d, (uint8_t*)(&d), 4);
    return d;
  }

  uint8_t Argument::GetByte()
  {
    if (dereferenceCount == 0)
      return data;
    uint32_t d = data;
    for (uint8_t i = dereferenceCount; i > 1; --i)
      ReadBytes(d, (uint8_t*)(&d), 4);
    uint8_t r = 0;
    ReadByte(d, r);
    return r;
  }

  Instruction::Instruction(std::string line)
    : id(0)
  {
    std::vector<std::string> split;
    while (line.size() > 0)
    {
      char commentChars[] = { ';', '|', '?', '}' };
      size_t comment = line.size();
      for (unsigned i = 0; i < 4; ++i)
        comment = std::min(comment, line.find_first_of(commentChars[i]));
      line = line.substr(0, comment);
      std::string newPart = line.substr(0, line.find(' '));
      line = line.substr(std::min(newPart.size() + 1, line.size()));
      if (newPart.size() > 0)
      {
        for (unsigned i = 0; i < newPart.size(); ++i)
          newPart[i] = std::toupper(newPart[i]);
        split.push_back(newPart);
      }
    }
    if (split.size() > 0)
    {
      id = GetInstructionIndex(split[0]);
      for (unsigned i = 1; i < split.size(); ++i)
      {
        Argument A;
        while (split[i][0] == '*') // Dereferences
        {
          ++A.dereferenceCount;
          split[i] = split[i].substr(1);
        }
        if (split[i][0] == '$') // Hex value
          A.data = std::stoi(split[i].substr(1), nullptr, 16);
        else if (split[i][0] == ':') // Binary value
          A.data = std::stoi(split[i].substr(1), nullptr, 2);
        else if (split[i][0] == '#') // Octal
          A.data = std::stoi(split[i].substr(1), nullptr, 8);
        else // Decimal value
          A.data = std::stoi(split[i], nullptr, 10);
        arguments.push_back(A);
      }
    }
  }

  Instruction::Instruction(std::string line, Context context)
    : id(0)
  {
    std::vector<std::string> split;
    while (line.size() > 0)
    {
      char commentChars[] = { ';', '|', '?', '}' };
      size_t comment = line.size();
      for (unsigned i = 0; i < 4; ++i)
        comment = std::min(comment, line.find_first_of(commentChars[i]));
      line = line.substr(0, comment);
      std::string newPart = line.substr(0, line.find(' '));
      line = line.substr(std::min(newPart.size() + 1, line.size()));
      if (newPart.size() > 0)
      {
        for (unsigned i = 0; i < newPart.size(); ++i)
          newPart[i] = std::toupper(newPart[i]);
        split.push_back(newPart);
      }
    }
    if (split.size() > 0)
    {
      id = GetInstructionIndex(split[0]);
      for (unsigned i = 1; i < split.size(); ++i)
      {
        Argument A;
        while (split[i][0] == '*') // Dereferences
        {
          ++A.dereferenceCount;
          split[i] = split[i].substr(1);
        }
        if (split[i][0] == '$') // Hex value
          A.data = std::stoi(split[i].substr(1), nullptr, 16);
        else if (split[i][0] == ':') // Binary value
          A.data = std::stoi(split[i].substr(1), nullptr, 2);
        else if (split[i][0] == '#') // Octal
          A.data = std::stoi(split[i].substr(1), nullptr, 8);
        else if (context.find(split[i]) != context.end()) // Label
          A.data = context[split[i]];
        else // Decimal value
          A.data = std::stoi(split[i], nullptr, 10);
        arguments.push_back(A);
      }
    }
  }


  InterpretResult::InterpretResult(bool success, std::string str)
    : success(success), str(str)
  {
  }

  InterpretResult::operator bool() const
  {
    return success;
  }

  InterpretResult InterpretLine(std::string line)
  {
    Instruction inst(line);
    if (inst.id)
    {
      if (inst.arguments.size() != instructionTable[inst.id].argumentCount)
        return InterpretResult(false, line.substr(0, line.find(' ')) + " requires " + std::to_string(instructionTable[inst.id].argumentCount) + " arguments.");
      return (inst.*(instructionTable[inst.id].function))(nullptr);
    }
    return InterpretResult(false, "Unknown instruction: " + line.substr(0, line.find(' ')));
  }

  std::vector<InterpretResult> InterpretLines(std::vector<std::string> lines)
  {
    Instruction::Context context;
    for (uint32_t i = 0; i < lines.size(); ++i)
    {
      std::string& line = lines[i];
      size_t p = line.find_first_not_of(' ');
      if (p != -1 && p != 0)
        line = line.substr(p);
      if (line.size() > 0 && line[0] == '>')
      {
        for (unsigned i = 0; i < line.size(); ++i)
          line[i] = std::toupper(line[i]);
        line = line.substr(1);
        p = line.find_first_not_of(' ');
        if (p != -1 && p != 0)
          line = line.substr(p);
        if (line.size() == 0)
        {
          std::vector<InterpretResult> results;
          results.push_back(InterpretResult(false, "Compilation error: No label name at line " + std::to_string(i)));
          return results;
        }
        std::string label = line.substr(0, line.find_first_of(' '));
        if (context.find(label) != context.end())
        {
          std::vector<InterpretResult> results;
          results.push_back(InterpretResult(false, "Compilation error: Redefinition of label \"" + line.substr(1) + "\" at line " + std::to_string(i)));
          return results;
        }
        line = line.substr(label.size());
        p = line.find_first_not_of(' ');
        if (p != -1 && p != 0)
          line = line.substr(p);
        int v = i + 1;
        if (line.size() > 0 && line[0] >= '0' && line[0] <= '9')
        {
          v = 0;
          for (size_t j = 0; j < line.size() && line[j] >= '0' && line[j] <= '9'; ++j)
            v = v * 10 + line[j] - '0';
        }
        context[label] = v;
        line = "";
      }
    }
    std::vector<Instruction> instructions;
    for (uint32_t i = 0; i < lines.size(); ++i)
    {
      std::string& line = lines[i];
      size_t p = line.find_first_not_of(' ');
      if (p != -1 && p != 0)
        line = line.substr(p);
      instructions.push_back(Instruction(line, context));
    }
    return InterpretInstructions(instructions, context);
  }

  std::vector<InterpretResult> InterpretInstructions(std::vector<Instruction> inst)
  {
    return InterpretInstructions(inst, Instruction::Context());
  }

  std::vector<InterpretResult> InterpretInstructions(std::vector<Instruction> inst, Instruction::Context context)
  {
    std::vector<InterpretResult> r;
    unsigned lnWidth = 0;
    for (size_t c = inst.size() + 1; c > 0; c /= 10)
      ++lnWidth;
    for (uint32_t i = 0; i < inst.size();)
    {
      Instruction& c = inst[i++];
      if (c.id == 0)
        continue;
      std::string ln = "";
      unsigned pad = lnWidth;
      for (unsigned c = i; c > 0 && pad > 0; c /= 10)
        --pad;
      ln.resize(pad, ' ');
      ln += std::to_string(i) + ": ";

      InterpretResult ir = (c.*(instructionTable[c.id].function))(&i);
      r.push_back(InterpretResult(ir.success, ln + ir.str));
      if (!r.back().success)
        break;
    }
    return r;
  }
}