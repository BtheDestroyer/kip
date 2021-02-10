#pragma once

#include "pch.h"

#include <string>

#include "kipUniversal.h"
#include "kipInstruction.h"

namespace kip
{
  DLLMODE bool MapMemory(Argument::Data* start, Argument::Address size, Argument::Address mappedStart);
  typedef void (*MemoryReadFunction)(Argument::Address offset, Argument::Data* out, Argument::Address count);
  typedef void (*MemoryWriteFunction)(Argument::Address offset, Argument::Data* in, Argument::Address count);
  DLLMODE bool MapMemory(MemoryReadFunction readFunc, MemoryWriteFunction writeFunc, Argument::Address size, Argument::Address mappedStart);
  DLLMODE bool UnmapMemory(Argument::Data* start);
  DLLMODE bool UnmapMemory(Argument::Address mappedStart);
  DLLMODE bool WriteByte(Argument::Address address, Argument::Data byte);
  DLLMODE bool ReadByte(Argument::Address address, Argument::Data& byte);
  DLLMODE bool WriteBytes(Argument::Address address, Argument::Data*bytes, Argument::Address count);
  DLLMODE bool ReadBytes(Argument::Address address, Argument::Data* bytes, Argument::Address count);
  DLLMODE bool WriteString(Argument::Address address, const std::string& string);
  DLLMODE bool ReadString(Argument::Address address, std::string& string);
  DLLMODE bool SetStackPointer(Argument::Address address);
  DLLMODE bool GetStackPointer(Argument::Address& address);
}
