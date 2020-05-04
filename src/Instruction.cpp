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
    // STR A B
    // Stores value of A at address B
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
    // FIL A B C
    // Fills value of A to addresses [B, B+C)
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
    // ADD A B C
    // Adds value of B to A and stores it at address C
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
    return InterpretResult(false, "Invalid instruction: " + inst.command);
  }
}
