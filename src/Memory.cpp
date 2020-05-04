#include "pch.h"

#include <algorithm>
#include <list>
#include <tuple>

#include "kipMemory.h"

namespace kip
{
  struct MemoryBlock {
    uint32_t mappedAddr;
    uint8_t* realAddr;
    uint32_t size;
    void (*readFunc)(uint32_t offset, uint8_t* out, uint32_t count);
    void (*writeFunc)(uint32_t offset, uint8_t* in, uint32_t count);
    enum Type {
      DATA,
      FUNC
    } type;
  };
  typedef std::list<MemoryBlock> MemoryMap;
  MemoryMap memoryMap;

  bool MapMemory(MemoryBlock newBlock)
  {
    if (newBlock.mappedAddr + newBlock.size <= newBlock.mappedAddr)
      return false; // Couldn't map. End position somehow before start position
    MemoryMap::iterator it = memoryMap.begin();
    if (it == memoryMap.end())
    {
      memoryMap.push_front(newBlock);
      return true; // Mapped to new first block
    }
    if (newBlock.mappedAddr + newBlock.size < it->mappedAddr)
    {
      memoryMap.push_front(newBlock);
      return true; // Mapped to new first block
    }
    while (it != memoryMap.end())
    {
      if (newBlock.mappedAddr < it->mappedAddr + it->size && newBlock.mappedAddr >= it->mappedAddr)
        return false; // Couldn't map. Start position has already been mapped
      if (newBlock.mappedAddr + newBlock.size < it->mappedAddr + it->size && newBlock.mappedAddr + newBlock.size >= it->mappedAddr)
        return false; // Couldn't map. End position has already been mapped
      if (newBlock.mappedAddr < it->mappedAddr && newBlock.mappedAddr + newBlock.size >= it->mappedAddr + it->size)
        return false; // Couldn't map. New block contains existing block
      MemoryMap::iterator prev = it++;
      if (it->mappedAddr > newBlock.mappedAddr + newBlock.size)
      {
        memoryMap.insert(prev, newBlock);
        return true; // Mapped to new middle block
      }
    }
    memoryMap.push_back(newBlock);
    return true; // Mapped to new end block
  }

  bool MapMemory(uint8_t* start, uint32_t size, uint32_t mappedStart)
  {
    return MapMemory({ mappedStart, start, size, nullptr, nullptr, MemoryBlock::DATA });
  }

  bool MapMemory(void (*readFunc)(uint32_t offset, uint8_t* out, uint32_t count), void (*writeFunc)(uint32_t offset, uint8_t* in, uint32_t count), uint32_t size, uint32_t mappedStart)
  {
    return MapMemory({ mappedStart, nullptr, size, readFunc, writeFunc, MemoryBlock::FUNC });
  }

  bool UnmapMemory(uint32_t mappedStart)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      if (it->mappedAddr == mappedStart)
      {
        memoryMap.erase(it);
        return true; // Unmapped memory
      }
      if (it->mappedAddr > mappedStart)
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
      if (it->realAddr == start)
      {
        memoryMap.erase(it);
        return true; // Unmapped memory
      }
      if (it->realAddr > start)
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
      if (it->mappedAddr <= address && it->mappedAddr + it->size > address)
      {
        uint32_t offset = address - it->mappedAddr;
        if (it->type == MemoryBlock::DATA)
          it->realAddr[offset] = byte;
        else if (it->type == MemoryBlock::FUNC)
          it->writeFunc(offset, &byte, 1);
        return true; // Memory found
      }
      if (it->mappedAddr > address)
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
      if (it->mappedAddr <= address && it->mappedAddr + it->size > address)
      {
        uint32_t offset = address - it->mappedAddr;
        if (it->type == MemoryBlock::DATA)
          byte = it->realAddr[offset];
        else if (it->type == MemoryBlock::FUNC)
          it->readFunc(offset, &byte, 1);
        return true; // Memory found
      }
      if (it->mappedAddr > address)
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
      if (it->mappedAddr <= address && it->mappedAddr + it->size > address)
      {
        uint32_t offset = address - it->mappedAddr;
        uint32_t toCopy = std::min(count, it->size - offset);
        if (it->type == MemoryBlock::DATA)
          std::memcpy(it->realAddr + offset, bytes, toCopy);
        else if (it->type == MemoryBlock::FUNC)
          it->writeFunc(offset, bytes, toCopy);
        count -= toCopy;
        address += toCopy;
        bytes += toCopy;
        if (count == 0)
          return true; // Coppied all data
      }
      if (it->mappedAddr > address)
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
      if (it->mappedAddr <= address && it->mappedAddr + it->size > address)
      {
        uint32_t offset = address - it->mappedAddr;
        uint32_t toCopy = std::min(count, it->size - offset);
        if (it->type == MemoryBlock::DATA)
          std::memcpy(bytes, it->realAddr + offset, toCopy);
        else if (it->type == MemoryBlock::FUNC)
          it->readFunc(offset, bytes, toCopy);
        count -= toCopy;
        address += toCopy;
        bytes += toCopy;
        if (count == 0)
          return true; // Coppied all data
      }
      if (it->mappedAddr > address)
        return false; // Memory was not mapped
      ++it;
    }
    return false; // Requested address was not mapped
  }
}
