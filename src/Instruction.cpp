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
    InterpretResult(Instruction::* function)(Instruction::Context*) const;
    uint8_t verbosity; // Lower numbers are higher priority
  } instructionTable[] = {
    { "", 0, nullptr, 255 },

    // Storage
    { "STB", 2, &Instruction::STB, 200 },
    { "STA", 2, &Instruction::STA, 200 },
    { "STS", 2, &Instruction::STS, 200 },
    { "FIL", 3, &Instruction::FIL, 150 },
    { "CPY", 3, &Instruction::CPY, 130 },
    { "PUB", 1, &Instruction::PUB, 120 },
    { "PUA", 1, &Instruction::PUA, 120 },
    { "PUS", 1, &Instruction::PUS, 120 },
    { "POB", 1, &Instruction::POB, 120 },
    { "POA", 1, &Instruction::POA, 120 },
    { "POS", 1, &Instruction::POS, 120 },
    { "BIN", 2, &Instruction::BIN, 120 },
    { "SAV", 3, &Instruction::SAV, 120 },

    // Debugging
    { "RDB", 1, &Instruction::RDB, 0   },
    { "RDA", 1, &Instruction::RDA, 0   },
    { "RDS", 1, &Instruction::RDS, 0   },

    // Control Flow
    { "JMP", 1, &Instruction::JMP, 100 },
    { "JEQ", 3, &Instruction::JEQ, 100 },
    { "JNE", 3, &Instruction::JNE, 100 },
    { "JGT", 3, &Instruction::JGT, 100 },
    { "JLT", 3, &Instruction::JLT, 100 },
    { "HLT", 0, &Instruction::HLT, 10  },
    { "CAL", 1, &Instruction::CAL, 80  },

    // Arithmetic
    { "ADB", 3, &Instruction::ADB, 150 },
    { "ADA", 3, &Instruction::ADA, 150 },
    { "SBB", 3, &Instruction::SBB, 150 },
    { "SBA", 3, &Instruction::SBA, 150 },
    { "MLB", 3, &Instruction::MLB, 150 },
    { "MLA", 3, &Instruction::MLA, 150 },
    { "DVB", 3, &Instruction::DVB, 150 },
    { "DVA", 3, &Instruction::DVA, 150 },
    { "MDA", 3, &Instruction::MDB, 150 },

    // Increment/decrement
    { "INB", 1, &Instruction::INB, 150 },
    { "INA", 1, &Instruction::INA, 150 },
    { "DCB", 1, &Instruction::DCB, 150 },
    { "DCA", 1, &Instruction::DCA, 150 },
    
    // Bit manipulation
    { "BLS", 3, &Instruction::BLS, 150 },
    { "BRS", 3, &Instruction::BRS, 150 },
    { "ROL", 3, &Instruction::ROL, 150 },
    { "ROR", 3, &Instruction::ROR, 150 },
    { "AND", 3, &Instruction::AND, 150 },
    { "BOR", 3, &Instruction::BOR, 150 },
    { "XOR", 3, &Instruction::XOR, 150 },
    { "NOT", 2, &Instruction::NOT, 150 },
  };

  uint8_t GetInstructionIndex(std::string instruction)
  {
    for (uint8_t i = 1; i < sizeof(instructionTable) / sizeof(*instructionTable); ++i)
      if (instruction == instructionTable[i].string)
        return i;
    return 0;
  }

  //////////////////////////////////////////////////////////////
  // Storage                 //
  /////////////////////////////

  InterpretResult Instruction::STB(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Address B = arguments[1].GetAddr();
    if (WriteByte(B, A))
      return InterpretResult(true, std::to_string(unsigned(B)) + "<=" + std::to_string(unsigned(A)));
    return InterpretResult(false, "Address " + std::to_string(B) + " not mapped");
  }

  InterpretResult Instruction::STA(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address B = arguments[1].GetAddr();
    if (WriteBytes(B, (uint8_t*)(&A), 4))
      return InterpretResult(true, std::to_string(unsigned(B)) + "<=" + std::to_string(unsigned(A)));
    return InterpretResult(false, "Address " + std::to_string(B) + " not mapped");
  }

  InterpretResult Instruction::STS(Context* context) const
  {
    std::string       A = arguments[0].GetString();
    Argument::Address B = arguments[1].GetAddr();
    if (WriteBytes(B, (uint8_t*)(A.data()), uint32_t(A.length() + 1)))
      return InterpretResult(true, std::to_string(unsigned(B)) + "<=" + A);
    return InterpretResult(false, "Address " + std::to_string(B) + " not mapped");
  }

  InterpretResult Instruction::FIL(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Address B = arguments[1].GetAddr();
    Argument::Address C = arguments[2].GetAddr();
    for (uint32_t i = B; i < B + C; ++i)
      if (!WriteByte(i, A))
        return InterpretResult(false, "Address " + std::to_string(i) + " not mapped");
    return InterpretResult(true, "[" + std::to_string(unsigned(B)) + ", " + std::to_string(unsigned(B + C)) + ")<=" + std::to_string(unsigned(A)));
  }

  InterpretResult Instruction::CPY(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address B = arguments[1].GetAddr();
    Argument::Address C = arguments[2].GetAddr();
    if (C > 0)
    {
      Bytecode::Data v;
      v.resize(C);
      if (!ReadBytes(A, v.data(), C))
        return InterpretResult(false, "An address in [" + std::to_string(unsigned(A)) + ", " + std::to_string(unsigned(A + C)) + ") is unmapped");
      if (!WriteBytes(B, v.data(), C))
        return InterpretResult(false, "An address in [" + std::to_string(unsigned(B)) + ", " + std::to_string(unsigned(B + C)) + ") is unmapped");
    }
    return InterpretResult(true, "[" + std::to_string(unsigned(B)) + "," + std::to_string(unsigned(B + C)) + ")<=[" + std::to_string(unsigned(A)) + ", " + std::to_string(unsigned(A + C)) + ")");
  }

  InterpretResult Instruction::PUB(Context* context) const
  {
    Argument::Address s = 0;
    Argument::Data    A = arguments[0].GetByte();
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (!WriteByte(s - 1, A))
      return InterpretResult(false, "Stack pointer is not mapped post-write (" + std::to_string(s - 1) + ")");
    if (!SetStackPointer(s - 1))
      return InterpretResult(false, "Stack pointer is not mapped post-decrement (" + std::to_string(s - 1) + ")");
    return InterpretResult(true, std::to_string(unsigned(s - 1)) + "<=" + std::to_string(unsigned(A)));
  }

  InterpretResult Instruction::PUA(Context* context) const
  {
    Argument::Address s = 0;
    Argument::Address A = arguments[0].GetAddr();
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (!WriteBytes(s - 4, (uint8_t*)(&A), 4))
      return InterpretResult(false, "Stack pointer is not mapped post-write (" + std::to_string(s - 4) + ")");
    if (!SetStackPointer(s - 4))
      return InterpretResult(false, "Stack pointer is not mapped post-decrement (" + std::to_string(s - 4) + ")");
    return InterpretResult(true, std::to_string(unsigned(s - 4)) + "<=" + std::to_string(unsigned(A)));
  }

  InterpretResult Instruction::PUS(Context* context) const
  {
    Argument::Address s = 0;
    std::string A = arguments[0].GetString();
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    Argument::Address size = Argument::Address(A.size());
    if (!WriteByte(s, 0))
      return InterpretResult(false, "Stack pointer is not mapped post-write (" + std::to_string(s - 4) + ")");
    if (!WriteBytes(s - size, (uint8_t*)(A.data()), size))
      return InterpretResult(false, "Stack pointer is not mapped post-write (" + std::to_string(s - 4) + ")");
    if (!SetStackPointer(s - size))
      return InterpretResult(false, "Stack pointer is not mapped post-decrement (" + std::to_string(s - 4) + ")");
    return InterpretResult(true, std::to_string(unsigned(s - size)) + "<=\"" + A + "\"");
  }

  InterpretResult Instruction::POB(Context* context) const
  {
    Argument::Address s = 0;
    Argument::Address A = arguments[0].GetAddr();
    Argument::Data    v = 0;
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (!ReadByte(s, v))
      return InterpretResult(false, "Stack pointer is not mapped post-read (" + std::to_string(s) + ")");
    if (!WriteByte(A, v))
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    if (!SetStackPointer(s + 1))
      return InterpretResult(false, "Stack pointer is not mapped post-increment (" + std::to_string(s + 1) + ")");
    return InterpretResult(true, std::to_string(unsigned(A)) + "<=" + std::to_string(unsigned(v)));
  }

  InterpretResult Instruction::POA(Context* context) const
  {
    Argument::Address s = 0;
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address v = 0;
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (!ReadBytes(s, (uint8_t*)(&v), 4))
      return InterpretResult(false, "Stack pointer is not mapped post-read (" + std::to_string(s) + ")");
    if (!WriteBytes(A, (uint8_t*)(&v), 4))
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    if (!SetStackPointer(s + 4))
      return InterpretResult(false, "Stack pointer is not mapped post-increment (" + std::to_string(s + 4) + ")");
    return InterpretResult(true, std::to_string(unsigned(A)) + "<=" + std::to_string(unsigned(v)));
  }

  InterpretResult Instruction::POS(Context* context) const
  {
    Argument::Address s = 0;
    Argument::Address A = arguments[0].GetAddr();
    std::string       str;
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    Argument::Data c = '\0';
    do
    {
      if (!ReadByte(s++, c))
        return InterpretResult(false, "Stack pointer is not mapped post-write (" + std::to_string(s - 4) + ")");
      if (c)
        str += char(c);
    } while (c);
    if (!SetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped post-decrement (" + std::to_string(s - 4) + ")");
    if (!WriteString(A, str))
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    return InterpretResult(true, std::to_string(unsigned(A)) + "<=\"" + str + "\"");
  }

  InterpretResult Instruction::BIN(Context* context) const
  {
    std::string       A = arguments[0].GetString();
    Argument::Address B = arguments[1].GetAddr();
    std::string       path;
    if (context && A.size() > 1 && A[0] == '.' && (A[1] == '/' || A[1] == '\\'))
      path = context->folder + A.substr(1);
    else
      path = A;
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
      return InterpretResult(false, "Could not open external file: " + A);
    const std::streamsize bufferSize = 128;
    char buffer[bufferSize] = { 0 };
    std::streamsize size = 0;
    Argument::Address addr = B;
    while (!file.eof())
    {
      file.read(buffer, bufferSize);
      Argument::Address size = Argument::Address(file.gcount());
      if (size > 0)
        size %= bufferSize;
      if (!WriteBytes(addr, (Argument::Data*)(buffer), size))
        return InterpretResult(false, "An address in [" + std::to_string(unsigned(addr)) + ", " + std::to_string(unsigned(addr + size)) + ") is unmapped");
      addr += size;
    }
    file.close();
    return InterpretResult(true, "[" + std::to_string(unsigned(B)) + "," + std::to_string(unsigned(addr)) + ")<={" + A + "}");
  }

  InterpretResult Instruction::SAV(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address B = arguments[1].GetAddr();
    std::string C = arguments[2].GetString();
    std::string path;
    if (context && C.size() > 1 && C[0] == '.' && (C[1] == '/' || C[1] == '\\'))
      path = context->folder + C.substr(1);
    else
      path = C;
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
      return InterpretResult(false, "Could not open external file: " + C);
    const std::streamsize bufferSize = 128;
    char buffer[bufferSize] = { 0 };
    Argument::Address addr = A;
    Argument::Address size = B;
    while (size > 0)
    {
      Argument::Address chunkSize = size > bufferSize ? bufferSize : size;
      if (!ReadBytes(addr, (Argument::Data*)(buffer), chunkSize))
        return InterpretResult(false, "An address in [" + std::to_string(unsigned(addr)) + ", " + std::to_string(unsigned(addr + size)) + ") is unmapped");
      file.write(buffer, chunkSize);
      addr += chunkSize;
      size -= chunkSize;
    }
    file.close();
    return InterpretResult(true, "[" + std::to_string(unsigned(A)) + "," + std::to_string(unsigned(addr)) + ")=>{" + C + "}");
  }

  /////////////////////////////
  // Debugging               //
  /////////////////////////////

  InterpretResult Instruction::RDB(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Data    out;
    if (ReadByte(A, out))
      return InterpretResult(true, std::to_string(unsigned(A)) + "=>" + std::to_string(int(out)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::RDA(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address out;
    if (ReadBytes(A, (Argument::Data*)(&out), sizeof(out)))
      return InterpretResult(true, std::to_string(unsigned(A)) + "=>" + std::to_string(int(out)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::RDS(Context* context) const
  {
    return InterpretResult(true, arguments[0].GetString());
  } 

  /////////////////////////////
  // Control Flow            //
  /////////////////////////////

  InterpretResult Instruction::JMP(Context* context) const
  {
    if (!context)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    context->line = arguments[0].GetAddr() - 1;
    return InterpretResult(true, "pc<=" + std::to_string(context->line));
  }

  InterpretResult Instruction::JEQ(Context* context) const
  {
    if (!context)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    Argument::Address A = arguments[0].GetAddr();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Data    C = arguments[2].GetByte();
    if (B == C)
    {
      context->line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(context->line));
  }

  InterpretResult Instruction::JNE(Context* context) const
  {
    if (!context)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    Argument::Address A = arguments[0].GetAddr();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Data    C = arguments[2].GetByte();
    if (B != C)
    {
      context->line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(context->line));
  }

  InterpretResult Instruction::JGT(Context* context) const
  {
    if (!context)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    Argument::Address A = arguments[0].GetAddr();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Data    C = arguments[2].GetByte();
    if (B > C)
    {
      context->line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(context->line));
  }

  InterpretResult Instruction::JLT(Context* context) const
  {
    if (!context)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    Argument::Address A = arguments[0].GetAddr();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Data    C = arguments[2].GetByte();
    if (B < C)
    {
      context->line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(context->line));
  }

  InterpretResult Instruction::JGE(Context* context) const
  {
    if (!context)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    Argument::Address A = arguments[0].GetAddr();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Data    C = arguments[2].GetByte();
    if (B >= C)
    {
      context->line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(context->line));
  }

  InterpretResult Instruction::JLE(Context* context) const
  {
    if (!context)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    Argument::Address A = arguments[0].GetAddr();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Data    C = arguments[2].GetByte();
    if (B <= C)
    {
      context->line = A - 1;
    }
    return InterpretResult(true, "pc<=" + std::to_string(context->line));
  }

  InterpretResult Instruction::HLT(Context* context) const
  {
    if (!context)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    context->line = uint32_t(-1);
    return InterpretResult(true, "Halted program");
  }

  InterpretResult Instruction::CAL(Context* context) const
  {
    Argument::Address s = 0;
    Argument::Address A = arguments[0].GetAddr();
    if (!context)
      return InterpretResult(false, std::string(instructionTable[id].string) + " cannot be run without context");
    Argument::Address next = context->line + 1;
    if (!GetStackPointer(s))
      return InterpretResult(false, "Stack pointer is not mapped");
    if (!WriteBytes(s - 4, (uint8_t*)(&next), 4))
      return InterpretResult(false, "Stack pointer is not mapped post-write (" + std::to_string(s - 4) + ")");
    if (!SetStackPointer(s - 4))
      return InterpretResult(false, "Stack pointer is not mapped post-decrement (" + std::to_string(s - 4) + ")");
    context->line = A - 1;
    return InterpretResult(true, std::to_string(unsigned(s - 4)) + "<=" + std::to_string(int(next)) + ";  pc <= " + std::to_string(context->line));
  }

  /////////////////////////////
  // Arithmetic              //
  /////////////////////////////

  InterpretResult Instruction::ADB(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    if (WriteByte(C, A + B))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(unsigned(A + B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::ADA(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address B = arguments[1].GetAddr();
    Argument::Address C = arguments[2].GetAddr();
    Argument::Address v = A + B;
    if (WriteBytes(C, (uint8_t*)&v, sizeof(v)))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(unsigned(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::SBB(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    if (WriteByte(C, A - B))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(unsigned(A - B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::SBA(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address B = arguments[1].GetAddr();
    Argument::Address C = arguments[2].GetAddr();
    Argument::Address v = A - B;
    if (WriteBytes(C, (uint8_t*)&v, sizeof(v)))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(unsigned(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::MLB(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    if (WriteByte(C, A * B))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(int(A * B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::MLA(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address B = arguments[1].GetAddr();
    Argument::Address C = arguments[2].GetAddr();
    Argument::Address v = A * B;
    if (WriteBytes(C, (uint8_t*)&v, sizeof(v)))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(unsigned(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::DVB(Context* context) const
  {
    Argument::Data  A = arguments[0].GetByte();
    Argument::Data  B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    if (WriteByte(C, A / B))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(int(A / B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::DVA(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address B = arguments[1].GetAddr();
    Argument::Address C = arguments[2].GetAddr();
    Argument::Address v = A / B;
    if (WriteBytes(C, (uint8_t*)&v, sizeof(v)))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(unsigned(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::MDB(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    if (WriteByte(C, A % B))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(int(A % B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::MDA(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address B = arguments[1].GetAddr();
    Argument::Address C = arguments[2].GetAddr();
    Argument::Address v = A % B;
    if (WriteBytes(C, (Argument::Data*)&v, sizeof(v)))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(unsigned(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  /////////////////////////////
  // Increment/decrement     //
  /////////////////////////////

  InterpretResult Instruction::INB(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Data    v;
    if (ReadByte(A, v) && WriteByte(A, ++v))
      return InterpretResult(true, std::to_string(unsigned(A)) + "<=" + std::to_string(unsigned(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::INA(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address v;
    if (ReadBytes(A, (Argument::Data*)(&v), sizeof(v)) && WriteBytes(A, (Argument::Data*)(&++v), sizeof(v)))
      return InterpretResult(true, std::to_string(unsigned(A)) + "<=" + std::to_string(unsigned(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::DCB(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Data    v;
    if (ReadByte(A, v) && WriteByte(A, --v))
      return InterpretResult(true, std::to_string(unsigned(A)) + "<=" + std::to_string(unsigned(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::DCA(Context* context) const
  {
    Argument::Address A = arguments[0].GetAddr();
    Argument::Address v;
    if (ReadBytes(A, (Argument::Data*)(&v), sizeof(v)) && WriteBytes(A, (Argument::Data*)(&--v), sizeof(v)))
      return InterpretResult(true, std::to_string(unsigned(A)) + "<=" + std::to_string(unsigned(v)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  /////////////////////////////
  // Bit manipulation        //
  /////////////////////////////

  InterpretResult Instruction::BLS(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    if (WriteByte(C, A << B))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(int(A << B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::BRS(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    if (WriteByte(C, A >> B))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(int(A >> B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::ROL(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    B %= 8;
    uint16_t r = uint16_t(A) << B;
    r = (r & 0x0F) | ((r & 0xF0) >> 8);
    if (WriteByte(C, Argument::Data(r)))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(unsigned(r)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::ROR(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    B %= 8;
    Argument::Data r = 0;
    r |= (A >> B) & 0xF;
    r |= A << (8 - B);
    if (WriteByte(C, Argument::Data(r)))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(unsigned(r)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::AND(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    if (WriteByte(C, Argument::Data(A & B)))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(int(A & B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::BOR(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    if (WriteByte(C, Argument::Data(A | B)))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(int(A | B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::XOR(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Data    B = arguments[1].GetByte();
    Argument::Address C = arguments[2].GetAddr();
    if (WriteByte(C, Argument::Data(A ^ B)))
      return InterpretResult(true, std::to_string(unsigned(C)) + "<=" + std::to_string(int(A ^ B)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  InterpretResult Instruction::NOT(Context* context) const
  {
    Argument::Data    A = arguments[0].GetByte();
    Argument::Address B = arguments[1].GetAddr();
    if (WriteByte(B, Argument::Data(~A)))
      return InterpretResult(true, std::to_string(unsigned(B)) + "<=" + std::to_string(int(~A)));
    return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
  }

  //////////////////////////////////////////////////////////////

  Argument::Argument()
    : type(Type::INVALID)
  {
  }

  Argument::Argument(Argument::AddressOrData data, Argument::Data dereferenceCount)
    : data(data), dereferenceCount(dereferenceCount), type(Type::DATA)
  {
  }

  Argument::Argument(const std::string& string)
    : stringLabel(string), type(Type::STRING)
  {
  }

  Argument::Address Argument::GetAddr() const
  {
    Argument::Address d = data;
    for (uint8_t i = dereferenceCount; i > 0; --i)
      if (!ReadBytes(d, (Argument::Data*)(&d), sizeof(d)))
        throw "Could not dereference address: " + std::to_string(d);
    return d;
  }

  Argument::Data Argument::GetByte() const
  {
    if (dereferenceCount == 0)
      return data;
    Argument::AddressOrData d = data;
    for (uint8_t i = dereferenceCount; i > 1; --i)
      if (!ReadBytes(d, (Argument::Data*)(&d), sizeof(d)))
        throw "Could not dereference address: " + std::to_string(d);
    Argument::Data r = 0;
    if (!ReadByte(d, r))
      throw "Could not read byte at address: " + std::to_string(d);
    return r;
  }

  const std::string Argument::GetString() const
  {
    if (type == Type::STRING)
    {
      if (dereferenceCount > 0)
        throw "String argument can't be dereferenced!";
      return stringLabel;
    }
    Argument::Address addr = data;
    for (uint8_t i = dereferenceCount; i > 0; --i)
      if (!ReadBytes(addr, (Argument::Data*)(&addr), sizeof(addr)))
        throw "Could not dereference address: " + std::to_string(addr);
    std::string r;
    if (!ReadString(addr, r))
      throw "Could not read string at address: " + std::to_string(addr);
    return r;
  }

  Instruction::Instruction()
    : line(), id(0)
  {
  }

  Instruction::Instruction(std::string line)
    : line(line), id(0)
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

  Instruction::Instruction(std::string line, Context& context)
    : line(line), id(0)
  {
    std::vector<std::string> split;
    while (line.size() > 0)
    {
      line = RemoveComments(line);
      std::string newPart;
      if (line[0] == '\"')
        newPart = line.substr(0, line.find('\"', 1) + 1);
      else
        newPart = line.substr(0, line.find(' '));
      line = line.substr(std::min(newPart.size() + 1, line.size()));
      if (newPart.size() > 0)
      {
        if (newPart[0] != '\"')
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
        Argument A(0);
        while (split[i][0] == '*') // Dereferences
        {
          ++A.dereferenceCount;
          split[i] = split[i].substr(1);
        }
        if (split[i][0] == '$') // Hex value
          A.data = std::stoul(split[i].substr(1), nullptr, 16);
        else if (split[i][0] == ':') // Binary value
          A.data = std::stoi(split[i].substr(1), nullptr, 2);
        else if (split[i][0] == '#') // Octal
          A.data = std::stoi(split[i].substr(1), nullptr, 8);
        else if (split[i][0] == '\"') // String literal
        {
          if (split[i].back() != '\"')
            throw "String literal argument did not have ending \"";
          A.stringLabel = split[i].substr(1, split[i].size() - 2);
          A.type = Argument::Type::STRING;
        }
        else if (context.labels.find(split[i]) != context.labels.end()) // Label
          A = context.labels[split[i]];
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
    try
    {
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
    }
    catch (std::exception e)
    {
      results.push_back(InterpretResult(false, "Exception was thrown while interpreting instructions: " + std::string(e.what())));
      return results;
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
        bool toUpper = true;
        for (unsigned i = 0; i < line.size(); ++i)
          if (line[i] == '\"')
            toUpper = !toUpper;
          else if (toUpper)
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
        context.labels[label] = v;
        if (line.size() > 0)
        {
          if (line[0] == '\"') // String
          {
            line = line.substr(1);
            p = line.find_last_of('\"');
            if (line.size() == 0)
            {
              results.push_back(InterpretResult(false, "Compilation error: No closing quotation for string literal label at line " + std::to_string(i)));
              return results;
            }
            line = line.substr(0, p);
            context.labels[label] = Argument(line);
          }
          else
          {
            if (context.labels.find(line) != context.labels.end()) // Label
              context.labels[label] = context.labels[line];
            else
            {
              if (line[0] == '$') // Hex value
                v = std::stoi(line.substr(1), nullptr, 16);
              else if (line[0] == ':') // Binary value
                v = std::stoi(line.substr(1), nullptr, 2);
              else if (line[0] == '#') // Octal
                v = std::stoi(line.substr(1), nullptr, 8);
              else // Decimal value
                v = std::stoi(line, nullptr, 10);
              context.labels[label] = v;
            }
          }
        }
        line = "";
      }
    }

    results.push_back(InterpretResult(true, "Built context labels successfully"));
    return results;
  }

  std::vector<Instruction> BuildInstructions(Instruction::Context& context, std::vector<std::string>& lines)
  {
    std::vector<Instruction> instructions;
    instructions.resize(lines.size());
    for (uint32_t i = 0; i < lines.size(); ++i)
    {
      std::string& line = lines[i];
      size_t p = line.find_first_not_of(' ');
      if (p != -1 && p != 0)
        line = line.substr(p);
      instructions.push_back(Instruction(line, context));
    }
    return instructions;
  }

  std::vector<InterpretResult> InterpretLines(std::vector<std::string> &lines, uint8_t verbosity)
  {
    return InterpretLines(lines, "", verbosity);
  }

  std::vector<InterpretResult> InterpretLines(std::vector<std::string> &lines, std::string folder, uint8_t verbosity)
  {
    Instruction::Context context;
    context.folder = folder;
    std::vector<InterpretResult> bcr = BuildContext(context, lines);
    if (!bcr.back().success)
      return bcr;
    
    std::vector<Instruction> instructions;
    try
    {
      instructions = BuildInstructions(context, lines);
    }
    catch (std::exception e)
    {
      std::vector<InterpretResult> results;
      results.push_back(InterpretResult(false, "Exception was thrown while building instructions: " + std::string(e.what())));
      return results;
    }
    return InterpretInstructions(instructions, context, verbosity);
  }

  std::vector<InterpretResult> InterpretInstructions(const std::vector<Instruction> &inst, uint8_t verbosity)
  {
    Instruction::Context c;
    return InterpretInstructions(inst, c, verbosity);
  }

  std::vector<InterpretResult> InterpretInstructions(const std::vector<Instruction> &inst, Instruction::Context &context, uint8_t verbosity)
  {
    std::vector<InterpretResult> r;
    if (verbosity > KIP_VERBOSITY_RESERVE_LARGE)
      r.reserve(100000);
    else if (verbosity > KIP_VERBOSITY_RESERVE_SMALL)
      r.reserve(5000);
    unsigned lnWidth = 0;
    for (size_t c = inst.size() + 1; c > 0; c /= 10)
      ++lnWidth;
    context.line = 0;
    if (context.labels.find("START") != context.labels.end())
      context.line = context.labels["START"].GetAddr();
    try
    {
      while (context.line < inst.size())
      {
        const Instruction& c = inst[context.line++];
        if (c.id == 0)
          continue;
        std::string ln = "";
        unsigned pad = lnWidth;
        for (unsigned c = context.line; c > 0 && pad > 0; c /= 10)
          --pad;
        ln.resize(pad, ' ');
        ln += std::to_string(context.line) + ": ";

        InterpretResult ir = (c.*(instructionTable[c.id].function))(&context);
        if (!ir.success)
        {
          r.push_back(InterpretResult(ir.success, ln + c.line + " -> " + ir.str));
          break;
        }
        else if (verbosity >= instructionTable[c.id].verbosity)
        {
          std::string newstr = ln + c.line + " -> " + ir.str;
          if (r.size() == 0 || r.back().str != newstr)
            r.push_back(InterpretResult(ir.success, ln + c.line + " -> " + ir.str));
        }
      }
    }
    catch (std::exception e)
    {
      r.push_back(InterpretResult(false, "Exception was thrown while interpreting instructions: " + std::string(e.what())));
      return r;
    }
    if (r.size() == 0 || r.back().success)
      r.push_back(InterpretResult(true, "Executed successfully"));
    return r;
  }

  Bytecode::Data CompileInstructionsToBytecode(const std::vector<Instruction>& inst, Instruction::Context& context)
  {
    Bytecode::Header header;
    return CompileInstructionsToBytecode(inst, context, header);
  }

  Bytecode::Data CompileInstructionsToBytecode(const std::vector<Instruction>& inst, Instruction::Context& context, Bytecode::Header& header)
  {
    Bytecode::Data bc;
    bc.resize(header.size());
    std::copy(header.begin(), header.end(), bc.begin());
    for (std::map<std::string, Argument>::iterator label = context.labels.begin(); label != context.labels.end(); ++label)
    {
      switch (label->second.type)
      {
      case Argument::Type::DATA:
        bc.push_back(uint8_t(Bytecode::DataType::LABEL_DATA));
        bc.resize(bc.size() + label->first.size() + 1);
        std::copy(label->first.begin(), label->first.end(), bc.end() - (label->first.size() + 1));
        bc.push_back(uint8_t(label->second.data >> 24));
        bc.push_back(uint8_t(label->second.data >> 16));
        bc.push_back(uint8_t(label->second.data >> 8 ));
        bc.push_back(uint8_t(label->second.data >> 0 ));
        break;
      case Argument::Type::STRING:
        bc.push_back(uint8_t(Bytecode::DataType::LABEL_STRING));
        bc.resize(bc.size() + label->first.size() + label->second.stringLabel.size() + 2);
        std::copy(label->first.begin(), label->first.end(), bc.end() - (label->first.size() + label->second.stringLabel.size() + 2));
        std::copy(label->second.stringLabel.begin(), label->second.stringLabel.end(), bc.end() - (label->second.stringLabel.size() + 1));
        break;
      }
    }
    for (const Instruction& i : inst)
    {
      uint8_t id = i.id + uint8_t(Bytecode::DataType::INSTRUCTIONS_START);
      if (id < uint8_t(Bytecode::DataType::INSTRUCTIONS_START)
        || id > uint8_t(Bytecode::DataType::INSTRUCTIONS_END))
        throw "Attempted to compile instruction id to a value outside of the valid range";
      bc.push_back(id);
      for (const Argument& a : i.arguments)
      {
        bc.push_back(uint8_t(a.type));
        switch (a.type)
        {
        case Argument::Type::DATA:
          bc.push_back(a.dereferenceCount);
          bc.push_back(uint8_t(a.data >> 24));
          bc.push_back(uint8_t(a.data >> 16));
          bc.push_back(uint8_t(a.data >> 8));
          bc.push_back(uint8_t(a.data >> 0));
          break;
        case Argument::Type::STRING:
          bc.resize(bc.size() + a.stringLabel.size() + 1);
          std::copy(a.stringLabel.begin(), a.stringLabel.end(), bc.end() - (a.stringLabel.size() + 1));
          break;
        }
      }
    }
    return bc;
  }

  Bytecode::Header BuildHeaderFromBytecode(const Bytecode::Data& inst, std::vector<InterpretResult>& r, uint32_t& offset)
  {
    Bytecode::Header header;
    if (inst.size() - offset < sizeof(header))
      r.push_back(InterpretResult(false, "Bytecode too short to contain a header."));
    else
    {
      memcpy(&header, inst.data() + offset, sizeof(header));
      offset += sizeof(header);
      r.push_back(InterpretResult(true, "Loaded header from bytecode."));
    }
    return header;
  }

  Instruction::Context BuildContextFromBytecode(const Bytecode::Data& inst, std::vector<InterpretResult>& r, const uint32_t& offset)
  {
    Instruction::Context context;
    try
    {
      uint32_t i = offset;
      for (InterpretResult sr : BuildContextImportsFromBytecode(context, inst, offset))
      {
        r.push_back(sr);
        if (!sr.success)
        {
          r.push_back(InterpretResult(false, "Failed to build context due to error in import parsing"));
          return context;
        }
      }
      for (InterpretResult sr : BuildContextLabelsFromBytecode(context, inst, offset))
      {
        r.push_back(sr);
        if (!sr.success)
        {
          r.push_back(InterpretResult(false, "Failed to build context due to error in label parsing"));
          return context;
        }
      }
    }
    catch (std::exception e)
    {
      r.push_back(InterpretResult(false, "Exception was thrown while interpreting instructions: " + std::string(e.what())));
      return context;
    }
    r.push_back(InterpretResult(true, "Built context successfully"));
    return context;
  }

  std::vector<InterpretResult> BuildContextImportsFromBytecode(Instruction::Context& context, const Bytecode::Data& inst, uint32_t offset)
  {
    std::vector<InterpretResult> r;
    return r;
  }

  std::vector<InterpretResult> BuildContextLabelsFromBytecode(Instruction::Context& context, const Bytecode::Data& inst, uint32_t offset)
  {
    std::vector<InterpretResult> r;
    while (offset < inst.size())
    {
      uint8_t id = inst[offset];
      if (id >= uint8_t(Bytecode::DataType::INSTRUCTIONS_START) && id <= uint8_t(Bytecode::DataType::INSTRUCTIONS_END))
      {
        // Skip current instruction
        offset += instructionTable[id - uint8_t(Bytecode::DataType::INSTRUCTIONS_START)].argumentCount * sizeof(uint32_t);
      }
      ++offset;
    }
    return r;
  }

  std::vector<InterpretResult> InterpretBytecode(const Bytecode::Data& bc, uint8_t verbosity)
  {
    std::vector<InterpretResult> r;
    uint32_t i = 0;
    Bytecode::Header header = BuildHeaderFromBytecode(bc, r, i);
    std::vector<Bytecode::Metadata> md;
    Instruction::Context context = BuildContextFromBytecode(bc, r, i);
    if (verbosity > KIP_VERBOSITY_RESERVE_LARGE)
      r.reserve(100000);
    else if (verbosity > KIP_VERBOSITY_RESERVE_SMALL)
      r.reserve(5000);
    unsigned lnWidth = 0;
    for (size_t c = bc.size() + 1; c > 0; c /= 10)
      ++lnWidth;
    context.line = 0;
    if (context.labels.find("START") != context.labels.end())
      context.line = context.labels["START"].GetAddr();
    try
    {
      while (i < bc.size())
      {
        uint8_t id = bc[i++];
        if (id >= uint8_t(Bytecode::DataType::INSTRUCTIONS_START) && id <= uint8_t(Bytecode::DataType::INSTRUCTIONS_END))
        {
        }
        else if (id == uint8_t(Bytecode::DataType::LABEL_DATA))
        {
          i += 4;
        }
        else if (id == uint8_t(Bytecode::DataType::LABEL_STRING))
        {
          i += 4;
        }
        else if (id == uint8_t(Bytecode::DataType::IMPORT))
        {
          i += 4;
        }
      }
    }
    catch (std::exception e)
    {
      r.push_back(InterpretResult(false, "Exception was thrown while interpreting instructions: " + std::string(e.what())));
      return r;
    }
    if (r.size() == 0 || r.back().success)
      r.push_back(InterpretResult(true, "Executed successfully"));
    return r;
  }
}
