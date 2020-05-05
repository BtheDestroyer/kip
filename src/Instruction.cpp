#include "pch.h"

#include <algorithm>
#include <string>
#include <vector>
#include <sstream>

#include "kipInstruction.h"
#include "kipMemory.h"

namespace kip
{
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
    return InterpretInstruction(ParseLine(line));
  }

  std::vector<InterpretResult> InterpretLines(std::vector<std::string> lines)
  {
    std::vector<Instruction> instructions;
    for (auto line : lines)
      instructions.push_back(ParseLine(line));
    return InterpretInstructions(instructions);
  }

  Instruction ParseLine(std::string line)
  {
    Instruction instruction;
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
      instruction.command = split[0];
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
        instruction.arguments.push_back(A);
      }
    }
    return instruction;
  }

  InterpretResult InterpretInstruction(Instruction inst)
  {
    if (inst.command.empty())
      return InterpretResult(true, "");
    if (inst.command == "JMP"
      || inst.command == "JEQ" || inst.command == "JNE"
      || inst.command == "JGT" || inst.command == "JGE"
      || inst.command == "JLT" || inst.command == "JLE"
      || inst.command == "HLT")
      return InterpretResult(false, "Cannot run contextless " + inst.command + " instruction");
    if (inst.command == "RDB")
    {
      if (inst.arguments.size() != 1)
        return InterpretResult(false, inst.command + " requires 1 arguments");
      uint32_t A = inst.arguments[0].GetAddr();
      uint8_t out;
      if (ReadByte(A, out))
        return InterpretResult(true, std::to_string(int(A)) + "=>" + std::to_string(int(out)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "RDA")
    {
      if (inst.arguments.size() != 1)
        return InterpretResult(false, inst.command + " requires 1 arguments");
      uint32_t A = inst.arguments[0].GetAddr();
      uint32_t out;
      if (ReadBytes(A, (uint8_t*)(&out), 4))
        return InterpretResult(true, std::to_string(int(A)) + "=>" + std::to_string(int(out)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "STR")
    {
      if (inst.arguments.size() != 2)
        return InterpretResult(false, inst.command + " requires 2 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint32_t B = inst.arguments[1].GetAddr();
      if (WriteByte(B, A))
        return InterpretResult(true, std::to_string(int(B)) + "<=" + std::to_string(int(A)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "FIL")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint32_t B = inst.arguments[1].GetAddr();
      uint32_t C = inst.arguments[2].GetAddr();
      for (uint32_t i = B; i < B + C; ++i)
        if (!WriteByte(i, A))
          return InterpretResult(false, "Address " + std::to_string(i) + " not mapped");
      return InterpretResult(true, "[" + std::to_string(int(B)) + ", " + std::to_string(int(B + C)) + ")<=" + std::to_string(int(A)));
    }
    if (inst.command == "ADD")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      if (WriteByte(C, A + B))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A + B)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "SUB")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      if (WriteByte(C, A - B))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A - B)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "MUL")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      if (WriteByte(C, A * B))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A * B)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "DIV")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      if (WriteByte(C, A / B))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A / B)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "MOD")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      if (WriteByte(C, A % B))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A % B)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "BLS")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      if (WriteByte(C, A << B))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A << B)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "BRS")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      if (WriteByte(C, A >> B))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A >> B)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "ROL")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      B %= 8;
      uint16_t r = uint16_t(A) << B;
      r = (r & 0x0F) | ((r & 0xF0) >> 8);
      if (WriteByte(C, uint8_t(r)))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(r)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "ROR")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      B %= 8;
      uint8_t r = 0;
      r |= (A >> B) & 0xF;
      r |= A << (8 - B);
      if (WriteByte(C, uint8_t(r)))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(r)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "AND")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      if (WriteByte(C, uint8_t(A & B)))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A & B)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "BOR")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      if (WriteByte(C, uint8_t(A | B)))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A | B)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "XOR")
    {
      if (inst.arguments.size() != 3)
        return InterpretResult(false, inst.command + " requires 3 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint8_t  B = inst.arguments[1].GetByte();
      uint32_t C = inst.arguments[2].GetAddr();
      if (WriteByte(C, uint8_t(A ^ B)))
        return InterpretResult(true, std::to_string(int(C)) + "<=" + std::to_string(int(A ^ B)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    if (inst.command == "NOT")
    {
      if (inst.arguments.size() != 2)
        return InterpretResult(false, inst.command + " requires 2 arguments");
      uint8_t  A = inst.arguments[0].GetByte();
      uint32_t B = inst.arguments[1].GetAddr();
      if (WriteByte(B, uint8_t(~A)))
        return InterpretResult(true, std::to_string(int(B)) + "<=" + std::to_string(int(~A)));
      return InterpretResult(false, "Address " + std::to_string(A) + " not mapped");
    }
    return InterpretResult(false, "Invalid instruction: " + inst.command);
  }

  std::vector<InterpretResult> InterpretInstructions(std::vector<Instruction> inst)
  {
    std::vector<InterpretResult> r;
    unsigned lnWidth = 0;
    for (size_t c = inst.size() + 1; c > 0; c /= 10)
      ++lnWidth;
    for (uint32_t i = 0; i < inst.size();)
    {
      std::string ln = "";
      unsigned pad = lnWidth;
      for (unsigned c = i + 1; c > 0 && pad > 0; c /= 10)
        --pad;
      ln.resize(pad, ' ');
      ln += std::to_string(i + 1) + ": ";

      if (inst[i].command == "JMP")
      {
        if (inst[i].arguments.size() != 1)
        {
          r.push_back(InterpretResult(false, ln + inst[i].command + " requires 1 arguments"));
          break;
        }
        i = inst[i].arguments[0].GetAddr() - 1;
        r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(i)));
      }
      else if (inst[i].command == "JEQ")
      {
        if (inst[i].arguments.size() != 3)
        {
          r.push_back(InterpretResult(false, ln + inst[i].command + " requires 3 arguments"));
          break;
        }
        uint32_t A = inst[i].arguments[0].GetAddr();
        uint8_t  B = inst[i].arguments[1].GetByte();
        uint8_t  C = inst[i].arguments[2].GetByte();
        if (B == C)
        {
          i = A - 1;
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(i)));
        }
        else
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(++i)));
      }
      else if (inst[i].command == "JNE")
      {
        if (inst[i].arguments.size() != 3)
        {
          r.push_back(InterpretResult(false, ln + inst[i].command + " requires 3 arguments"));
          break;
        }
        uint32_t A = inst[i].arguments[0].GetAddr();
        uint8_t  B = inst[i].arguments[1].GetByte();
        uint8_t  C = inst[i].arguments[2].GetByte();
        if (B != C)
        {
          i = A - 1;
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(i)));
        }
        else
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(++i)));
      }
      else if (inst[i].command == "JGT")
      {
        if (inst[i].arguments.size() != 3)
        {
          r.push_back(InterpretResult(false, ln + inst[i].command + " requires 3 arguments"));
          break;
        }
        uint32_t A = inst[i].arguments[0].GetAddr();
        uint8_t  B = inst[i].arguments[1].GetByte();
        uint8_t  C = inst[i].arguments[2].GetByte();
        if (B > C)
        {
          i = A - 1;
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(i)));
        }
        else
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(++i)));
      }
      else if (inst[i].command == "JLT")
      {
        if (inst[i].arguments.size() != 3)
        {
          r.push_back(InterpretResult(false, ln + inst[i].command + " requires 3 arguments"));
          break;
        }
        uint32_t A = inst[i].arguments[0].GetAddr();
        uint8_t  B = inst[i].arguments[1].GetByte();
        uint8_t  C = inst[i].arguments[2].GetByte();
        if (B < C)
        {
          i = A - 1;
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(i)));
        }
        else
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(++i)));
      }
      else if (inst[i].command == "JGE")
      {
        if (inst[i].arguments.size() != 3)
        {
          r.push_back(InterpretResult(false, ln + inst[i].command + " requires 3 arguments"));
          break;
        }
        uint32_t A = inst[i].arguments[0].GetAddr();
        uint8_t  B = inst[i].arguments[1].GetByte();
        uint8_t  C = inst[i].arguments[2].GetByte();
        if (B >= C)
        {
          i = A - 1;
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(i)));
        }
        else
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(++i)));
      }
      else if (inst[i].command == "JLE")
      {
        if (inst[i].arguments.size() != 3)
        {
          r.push_back(InterpretResult(false, ln + inst[i].command + " requires 3 arguments"));
          break;
        }
        uint32_t A = inst[i].arguments[0].GetAddr();
        uint8_t  B = inst[i].arguments[1].GetByte();
        uint8_t  C = inst[i].arguments[2].GetByte();
        if (B <= C)
        {
          i = A - 1;
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(i)));
        }
        else
          r.push_back(InterpretResult(true, ln + "pc<=" + std::to_string(++i)));
      }
      else if (inst[i].command == "HLT")
      {
        if (inst[i].arguments.size() != 0)
        {
          r.push_back(InterpretResult(false, ln + inst[i].command + " requires 0 arguments"));
          break;
        }
        r.push_back(InterpretResult(true, ln + "Halted program"));
        break;
      }
      else
      {
        InterpretResult ir = InterpretInstruction(inst[i]);
        r.push_back(InterpretResult(ir.success, ln + ir.str));
        if (!r.back().success)
          break;
        ++i;
      }
    }
    return r;
  }
}
