#include "pch.h"

#include <algorithm>
#include <list>
#include <tuple>
#include <string>

#include "kipMemory.h"

namespace kip
{
  struct MemoryBlock {
    uint32_t mappedAddr;
    uint8_t* realAddr;
    uint32_t size;
    void (*readFunc)(uint32_t offset, uint8_t* out, uint32_t count);
    void (*writeFunc)(uint32_t offset, uint8_t* in, uint32_t count);
    enum class Type {
      DATA,
      FUNC
    } type;
  };
  typedef std::list<MemoryBlock> MemoryMap;
  MemoryMap memoryMap;
  uint32_t stackPointer;

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
      ++it;
      if (it != memoryMap.end() && it->mappedAddr >= newBlock.mappedAddr + newBlock.size)
      {
        memoryMap.insert(it, newBlock);
        return true; // Mapped to new middle block
      }
    }
    memoryMap.push_back(newBlock);
    return true; // Mapped to new end block
  }

  bool MapMemory(uint8_t* start, uint32_t size, uint32_t mappedStart)
  {
    return MapMemory({ mappedStart, start, size, nullptr, nullptr, MemoryBlock::Type::DATA });
  }

  bool MapMemory(void (*readFunc)(uint32_t offset, uint8_t* out, uint32_t count), void (*writeFunc)(uint32_t offset, uint8_t* in, uint32_t count), uint32_t size, uint32_t mappedStart)
  {
    return MapMemory({ mappedStart, nullptr, size, readFunc, writeFunc, MemoryBlock::Type::FUNC });
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
        if (it->type == MemoryBlock::Type::DATA)
          it->realAddr[offset] = byte;
        else if (it->type == MemoryBlock::Type::FUNC)
        {
          if (it->writeFunc)
            it->writeFunc(offset, &byte, 1);
          else
            return false; // Memory is read-only
        }
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
        if (it->type == MemoryBlock::Type::DATA)
          byte = it->realAddr[offset];
        else if (it->type == MemoryBlock::Type::FUNC)
        {
          if (it->readFunc)
            it->readFunc(offset, &byte, 1);
          else
            return false; // Memory is write-only
        }
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
        if (it->type == MemoryBlock::Type::DATA)
          std::memcpy(it->realAddr + offset, bytes, toCopy);
        else if (it->type == MemoryBlock::Type::FUNC)
        {
          if (it->writeFunc)
            it->writeFunc(offset, bytes, 1);
          else
            return false; // Memory is read-only
        }
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
        if (it->type == MemoryBlock::Type::DATA)
          std::memcpy(bytes, it->realAddr + offset, toCopy);
        else if (it->type == MemoryBlock::Type::FUNC)
        {
          if (it->readFunc)
            it->readFunc(offset, bytes, 1);
          else
            return false; // Memory is write-only
        }
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

  bool WriteString(uint32_t address, const std::string& string)
  {
    return WriteBytes(address, (uint8_t*)(string.data()), uint32_t(string.length() + 1));
  }

  bool ReadString(uint32_t address, std::string& string)
  {
    string.clear();
    uint8_t c = '\0';
    do
    {
      if (!ReadByte(address++, c))
        return false;
      string += char(c);
    } while (c);
    return true;
  }

  bool SetStackPointer(uint32_t address)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      if (it->mappedAddr <= address && it->mappedAddr + it->size >= address)
      {
        stackPointer = address;
        return true; // Memory is mapped
      }
      if (it->mappedAddr > address)
        return false; // Memory was not mapped
      ++it;
    }
    return false; // Requested address was not mapped
  }

  bool GetStackPointer(uint32_t& address)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      if (it->mappedAddr <= stackPointer && it->mappedAddr + it->size >= stackPointer)
      {
        address = stackPointer;
        return true; // Memory is mapped
      }
      if (it->mappedAddr > address)
        return false; // Memory was not mapped
      ++it;
    }
    return false; // Requested address was not mapped
  }
}
