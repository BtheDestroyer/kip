#include "pch.h"

#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
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
    { "ADB", 3, &Instruction::ADB },
    { "ADA", 3, &Instruction::ADA },
    { "SBB", 3, &Instruction::SBB },
    { "SBA", 3, &Instruction::SBA },
    { "JMP", 1, &Instruction::JMP },
    { "JEQ", 3, &Instruction::JEQ },
    { "JNE", 3, &Instruction::JNE },
    { "JGT", 3, &Instruction::JGT },
    { "JLT", 3, &Instruction::JLT },
    { "MLB", 3, &Instruction::MLB },
    { "MLA", 3, &Instruction::MLA },
    { "DVB", 3, &Instruction::DVB },
    { "DVA", 3, &Instruction::DVA },
    { "MDA", 3, &Instruction::MDB },
    { "BLS", 3, &Instruction::BLS },
    { "BRS", 3, &Instruction::BRS },
    { "ROL", 3, &Instruction::ROL },
    { "ROR", 3, &Instruction::ROR },
    { "AND", 3, &Instruction::AND },
    { "BOR", 3, &Instruction::BOR },
    { "XOR", 3, &Instruction::XOR },
    { "NOT", 2, &Instruction::NOT },
    { "HLT", 0, &Instruction::HLT },
    { "INB", 1, &Instruction::INB },
    { "INA", 1, &Instruction::INA },
    { "DCB", 1, &Instruction::DCB },
    { "DCA", 1, &Instruction::DCA },
    { "PUB", 1, &Instruction::PUB },
    { "PUA", 1, &Instruction::PUA },
    { "POB", 1, &Instruction::POB },
    { "POA", 1, &Instruction::POA },
    { "CPY", 3, &Instruction::CPY },
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

  InterpretResult Instruction::ADB(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A + B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A + B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::ADA(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint32_t B = arguments[1].GetAddr();
    uint32_t C = arguments[2].GetAddr();
    uint32_t v = A + B;
    if (WriteBytes(C, (uint8_t*)&v, 4))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::SBB(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A - B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A - B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::SBA(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint32_t B = arguments[1].GetAddr();
    uint32_t C = arguments[2].GetAddr();
    uint32_t v = A - B;
    if (WriteBytes(C, (uint8_t*)&v, 4))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(v)));
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

  InterpretResult Instruction::MLB(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A * B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A * B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::MLA(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint32_t B = arguments[1].GetAddr();
    uint32_t C = arguments[2].GetAddr();
    uint32_t v = A * B;
    if (WriteBytes(C, (uint8_t*)&v, 4))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::DVB(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A / B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A / B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::DVA(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint32_t B = arguments[1].GetAddr();
    uint32_t C = arguments[2].GetAddr();
    uint32_t v = A / B;
    if (WriteBytes(C, (uint8_t*)&v, 4))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::MDB(uint32_t* line)
  {
    uint8_t  A = arguments[0].GetByte();
    uint8_t  B = arguments[1].GetByte();
    uint32_t C = arguments[2].GetAddr();
    if (WriteByte(C, A % B))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A % B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::MDA(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint32_t B = arguments[1].GetAddr();
    uint32_t C = arguments[2].GetAddr();
    uint32_t v = A % B;
    if (WriteBytes(C, (uint8_t*)&v, 4))
      return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(v)));
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

  InterpretResult Instruction::INB(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint8_t  v;
    if (ReadByte(A, v) && WriteByte(A, ++v))
      return InterpretResult(true, std::to_string(int(A)) + "<=" + std::to_string(int(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::INA(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint32_t v;
    if (ReadBytes(A, (uint8_t*)(&v), 4) && WriteBytes(A, (uint8_t*)(&++v), 4))
      return InterpretResult(true, std::to_string(int(A)) + "<=" + std::to_string(int(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::DCB(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint8_t  v;
    if (ReadByte(A, v) && WriteByte(A, --v))
      return InterpretResult(true, std::to_string(int(A)) + "<=" + std::to_string(int(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::DCA(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint32_t v;
    if (ReadBytes(A, (uint8_t*)(&v), 4) && WriteBytes(A, (uint8_t*)(&--v), 4))
      return InterpretResult(true, std::to_string(int(A)) + "<=" + std::to_string(int(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::PUB(uint32_t* line)
  {
    uint32_t s = 0;
    uint8_t  A = arguments[0].GetByte();
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (WriteByte(s - 1, A))
    {
      if (!SetStackPointer(s - 1))
        return InterpretResult(false, "Stack pointer is not mapped post-decrement (" + std::to_string(s - 1) + ")");
      return InterpretResult(true, std::to_string(int(s - 1)) + "<=" + std::to_string(int(A)));
    }
    return InterpretResult(false, "Stack pointer is not mapped post-write (" + std::to_string(s - 1) + ")");
  }

  InterpretResult Instruction::PUA(uint32_t* line)
  {
    uint32_t s = 0;
    uint32_t A = arguments[0].GetAddr();
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (WriteBytes(s - 4, (uint8_t*)(&A), 4))
    {
      if (!SetStackPointer(s - 4))
        return InterpretResult(false, "Stack pointer is not mapped post-decrement (" + std::to_string(s - 4) + ")");
      return InterpretResult(true, std::to_string(int(s - 4)) + "<=" + std::to_string(int(A)));
    }
    return InterpretResult(false, "Stack pointer is not mapped post-write (" + std::to_string(s - 4) + ")");
  }

  InterpretResult Instruction::POB(uint32_t* line)
  {
    uint32_t s = 0;
    uint32_t A = arguments[0].GetAddr();
    uint8_t  v = 0;
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (ReadByte(s, v))
    {
      if (!WriteByte(A, v))
        return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
      if (!SetStackPointer(s + 1))
        return InterpretResult(false, "Stack pointer is not mapped post-increment (" + std::to_string(s + 1) + ")");
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
    if (ReadBytes(s, (uint8_t*)(&v), 4))
    {
      if (!WriteBytes(A, (uint8_t*)(&v), 4))
        return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
      if (!SetStackPointer(s + 4))
        return InterpretResult(false, "Stack pointer is not mapped post-increment (" + std::to_string(s + 4) + ")");
      return InterpretResult(true, std::to_string(int(A)) + "<=" + std::to_string(int(v)));
    }
    return InterpretResult(false, "Stack pointer is not mapped post-read (" + std::to_string(s) + ")");
  }

  InterpretResult Instruction::CPY(uint32_t* line)
  {
    uint32_t A = arguments[0].GetAddr();
    uint32_t B = arguments[1].GetAddr();
    uint8_t  C = arguments[2].GetByte();
    if (C > 0)
    {
      std::vector<uint8_t> v;
      v.resize(C);
      if (!ReadBytes(A, v.data(), C))
        return InterpretResult(false, "An address in [" + std::to_string(int(A)) + ", " + std::to_string(int(A + C)) + ") is unmapped");
      if (!WriteBytes(B, v.data(), C))
        return InterpretResult(false, "An address in [" + std::to_string(int(B)) + ", " + std::to_string(int(B + C)) + ") is unmapped");
    }
    return InterpretResult(true, "[" + std::to_string(int(B)) + "," + std::to_string(int(B + C)) + ")<=[" + std::to_string(int(A)) + ", " + std::to_string(int(A + C)) + ")");
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
      line = RemoveComments(line);
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
        else if (context.labels.find(split[i]) != context.labels.end()) // Label
          A.data = context.labels[split[i]];
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

  std::string RemoveComments(std::string line)
  {
    char commentChars[] = { ';', '|', '?', '}' };
    size_t comment = line.size();
    for (unsigned i = 0; i < 4; ++i)
      comment = std::min(comment, line.find_first_of(commentChars[i]));
    return line.substr(0, comment);
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

  InterpretResult LoadFile(std::string filename, std::vector<std::string>& lines)
  {
    std::ifstream file(filename);
    if (file.is_open())
    {
      char line[2048];
      while (!file.eof())
      {
        file.getline(line, 2048);
        lines.push_back(line);
      }
      file.close();
      return InterpretResult(true, "Loaded " + filename);
    }
    return InterpretResult(false, "Could not open file " + filename);
  }

  std::vector<InterpretResult> BuildContext(Instruction::Context& context, std::vector<std::string>& lines)
  {
    std::vector<InterpretResult> results;
    for (InterpretResult sr : BuildContextImports(context, lines))
    {
      results.push_back(sr);
      if (!sr.success)
      {
        results.push_back(InterpretResult(false, "Failed to build context due to error in import parsing"));
        return results;
      }
    }
    for (InterpretResult sr : BuildContextLabels(context, lines))
    {
      results.push_back(sr);
      if (!sr.success)
      {
        results.push_back(InterpretResult(false, "Failed to build context due to error in label parsing"));
        return results;
      }
    }
    results.push_back(InterpretResult(true, "Built context successfully"));
    return results;
  }

  std::vector<InterpretResult> BuildContextImports(Instruction::Context& context, std::vector<std::string>& lines)
  {
    std::vector<InterpretResult> results;

    int i = 1;
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it, ++i)
    {
      std::string& line = *it;
      size_t p = line.find_first_not_of(' ');
      if (p != -1 && p != 0)
        line = line.substr(p);
      if (line.size() > 0 && line[0] == '<')
      {
        line = line.substr(1);
        p = line.find_first_not_of(' ');
        if (p != -1 && p != 0)
          line = line.substr(p);
        if (line.size() == 0)
        {
          results.push_back(InterpretResult(false, "Compilation error: No import filename at line " + std::to_string(i)));
          return results;
        }
        std::vector<std::string> newlines;
        results.push_back(LoadFile(context.folder + "\\" + line, newlines));
        if (!results.back().success)
          return results;
        line = "";
        if (newlines.size() > 0)
        {
          lines.insert(it, newlines.begin(), newlines.end());
          it = lines.begin();
          i = 0;
        }
      }
    }

    results.push_back(InterpretResult(true, "Imported files successfully"));
    return results;
  }

  std::vector<InterpretResult> BuildContextLabels(Instruction::Context& context, std::vector<std::string>& lines)
  {
    std::vector<InterpretResult> results;
    context.labels.clear();

    for (uint32_t i = 0; i < lines.size(); ++i)
    {
      std::string& line = lines[i];
      line = RemoveComments(line);
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
          results.push_back(InterpretResult(false, "Compilation error: No label name at line " + std::to_string(i)));
          return results;
        }
        std::string label = line.substr(0, line.find_first_of(' '));
        line = line.substr(label.size());
        p = line.find_first_not_of(' ');
        if (p == -1)
          line = "";
        else if (p != 0)
          line = line.substr(p);
        int v = i + 1;
        if (line.size() > 0)
        {
          if (line[0] == '$') // Hex value
            v = std::stoi(line.substr(1), nullptr, 16);
          else if (line[0] == ':') // Binary value
            v = std::stoi(line.substr(1), nullptr, 2);
          else if (line[0] == '#') // Octal
            v = std::stoi(line.substr(1), nullptr, 8);
          else if (context.labels.find(line) != context.labels.end()) // Label
            v = context.labels[line];
          else // Decimal value
            v = std::stoi(line, nullptr, 10);
        }
        context.labels[label] = v;
        line = "";
      }
    }

    results.push_back(InterpretResult(true, "Built context labels successfully"));
    return results;
  }

  std::vector<InterpretResult> InterpretLines(std::vector<std::string> lines)
  {
    return InterpretLines(lines, "");
  }

  std::vector<InterpretResult> InterpretLines(std::vector<std::string> lines, std::string folder)
  {
    Instruction::Context context;
    context.folder = folder;
    std::vector<InterpretResult> bcr = BuildContext(context, lines);
    if (!bcr.back().success)
      return bcr;
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
    uint32_t i = 0;
    if (context.labels.find("START") != context.labels.end())
      i = context.labels["START"];
    for (; i < inst.size();)
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
