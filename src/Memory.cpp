#include "pch.h"

#include <algorithm>
#include <list>
#include <tuple>

#include "kipMemory.h"

namespace kip
{
  // List of                mapped addr, real addr,     size
  typedef std::list<std::tuple<uint32_t,  uint8_t*, uint32_t>> MemoryMap;
  MemoryMap memoryMap;

  bool MapMemory(uint8_t* start, uint32_t size, uint32_t mappedStart)
  {
    if (mappedStart + size <= mappedStart)
      return false; // Couldn't map. End position somehow before start position
    MemoryMap::iterator it = memoryMap.begin();
    if (it == memoryMap.end())
    {
      memoryMap.push_front(std::make_tuple(mappedStart, start, size));
      return true; // Mapped to new first block
    }
    uint32_t ma, s;
    uint8_t* ra;
    std::tie(ma, ra, s) = *it;
    if (mappedStart + size < ma)
    {
      memoryMap.push_front(std::make_tuple(mappedStart, start, size));
      return true; // Mapped to new first block
    }
    while (it != memoryMap.end())
    {
      std::tie(ma, ra, s) = *it;
      if (mappedStart < ma + s && mappedStart >= ma)
        return false; // Couldn't map. Start position has already been mapped
      if (mappedStart + size < ma + s && mappedStart + size >= ma)
        return false; // Couldn't map. End position has already been mapped
      if (mappedStart < ma && mappedStart + size >= ma + s)
        return false; // Couldn't map. New block contains existing block
      MemoryMap::iterator prev = it++;
      uint32_t ma2, s2;
      uint8_t* ra2;
      std::tie(ma2, ra2, s2) = *it;
      if (ma2 > mappedStart + size)
      {
        memoryMap.insert(prev, std::make_tuple(mappedStart, start, size));
        return true; // Mapped to new middle block
      }
    }
    memoryMap.push_back(std::make_tuple(mappedStart, start, size));
    return true; // Mapped to new end block
  }

  bool UnmapMemory(uint32_t mappedStart)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      uint32_t ma, s;
      uint8_t* ra;
      std::tie(ma, ra, s) = *it;
      if (ma == mappedStart)
      {
        memoryMap.erase(it);
        return true; // Unmapped memory
      }
      if (ma > mappedStart)
      {
        return false; // Memory was not mapped
      }
      ++it;
    }
    return false; // Memory was not mapped
  }

  bool UnmapMemory(uint8_t* start)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      uint32_t ma, s;
      uint8_t* ra;
      std::tie(ma, ra, s) = *it;
      if (ra == start)
      {
        memoryMap.erase(it);
        return true; // Unmapped memory
      }
      if (ra > start)
        return false; // Memory was not mapped
      ++it;
    }
    return false; // Memory was not mapped
  }

  bool WriteByte(uint32_t address, uint8_t byte)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      uint32_t ma, s;
      uint8_t* ra;
      std::tie(ma, ra, s) = *it;
      if (ma <= address && ma + s > address)
      {
        ra[address - ma] = byte;
        return true; // Memory found
      }
      if (ma > address)
        return false; // Memory was not mapped
      ++it;
    }
    return false; // Requested address was not mapped
  }

  bool ReadByte(uint32_t address, uint8_t& byte)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      uint32_t ma, s;
      uint8_t* ra;
      std::tie(ma, ra, s) = *it;
      if (ma <= address && ma + s > address)
      {
        byte = ra[address - ma];
        return true; // Memory found
      }
      if (ma > address)
        return false; // Memory was not mapped
      ++it;
    }
    return false; // Requested address was not mapped
  }

  bool WriteBytes(uint32_t address, uint8_t* bytes, uint32_t count)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      uint32_t ma, s;
      uint8_t* ra;
      std::tie(ma, ra, s) = *it;
      if (ma <= address && ma + s > address)
      {
        uint32_t addrOffset = address - ma;
        uint32_t toCopy = std::min(count, s - addrOffset);
        std::memcpy(ra + addrOffset, bytes, toCopy);
        count -= toCopy;
        address += toCopy;
        bytes += toCopy;
        if (count == 0)
          return true; // Coppied all data
      }
      if (ma > address)
        return false; // Memory was not mapped
      ++it;
    }
    return false; // Requested address was not mapped
  }

  bool ReadBytes(uint32_t address, uint8_t* bytes, uint32_t count)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      uint32_t ma, s;
      uint8_t* ra;
      std::tie(ma, ra, s) = *it;
      if (ma <= address && ma + s > address)
      {
        uint32_t addrOffset = address - ma;
        uint32_t toCopy = std::min(count, s - addrOffset);
        std::memcpy(bytes, ra + addrOffset, toCopy);
        count -= toCopy;
        address += toCopy;
        bytes += toCopy;
        if (count == 0)
          return true; // Coppied all data
      }
      if (ma > address)
        return false; // Memory was not mapped
      ++it;
    }
    return false; // Requested address was not mapped
  }
}
