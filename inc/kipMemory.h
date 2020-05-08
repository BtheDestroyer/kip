#pragma once

#include "pch.h"
#include "kipUniversal.h"

namespace kip
{
  DLLMODE bool MapMemory(uint8_t* start, uint32_t size, uint32_t mappedStart);
  DLLMODE bool MapMemory(void (*readFunc)(uint32_t offset, uint8_t* out, uint32_t count), void (*writeFunc)(uint32_t offset, uint8_t* in, uint32_t count), uint32_t size, uint32_t mappedStart);
  DLLMODE bool UnmapMemory(uint8_t* start);
  DLLMODE bool UnmapMemory(uint32_t mappedStart);
  DLLMODE bool WriteByte(uint32_t address, uint8_t byte);
  DLLMODE bool ReadByte(uint32_t address, uint8_t& byte);
  DLLMODE bool WriteBytes(uint32_t address, uint8_t *bytes, uint32_t count);
  DLLMODE bool ReadBytes(uint32_t address, uint8_t* bytes, uint32_t count);
  DLLMODE bool SetStackPointer(uint32_t address);
  DLLMODE bool GetStackPointer(uint32_t& address);
}
