#include "pch.h"

#include <algorithm>
#include <list>
#include <tuple>
#include <string>

#include "kipMemory.h"

namespace kip
{
  struct MemoryBlock {
    Argument::Address mappedAddr;
    Argument::Data* realAddr;
    Argument::Address size;
    MemoryReadFunction readFunc;
    MemoryWriteFunction writeFunc;
    enum class Type {
      DATA,
      FUNC
    } type;
  };
  typedef std::list<MemoryBlock> MemoryMap;
  MemoryMap memoryMap;
  Argument::Address stackPointer;

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

  bool MapMemory(Argument::Data* start, Argument::Address size, Argument::Address mappedStart)
  {
    return MapMemory({ mappedStart, start, size, nullptr, nullptr, MemoryBlock::Type::DATA });
  }

  bool MapMemory(MemoryReadFunction readFunc, MemoryWriteFunction writeFunc, Argument::Address size, Argument::Address mappedStart)
  {
    return MapMemory({ mappedStart, nullptr, size, readFunc, writeFunc, MemoryBlock::Type::FUNC });
  }

  bool UnmapMemory(Argument::Address mappedStart)
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

  bool UnmapMemory(Argument::Data* start)
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

  bool WriteByte(Argument::Address address, Argument::Data byte)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      if (it->mappedAddr <= address && it->mappedAddr + it->size > address)
      {
        Argument::Address offset = address - it->mappedAddr;
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

  bool ReadByte(Argument::Address address, Argument::Data& byte)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      if (it->mappedAddr <= address && it->mappedAddr + it->size > address)
      {
        Argument::Address offset = address - it->mappedAddr;
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

  bool WriteBytes(Argument::Address address, Argument::Data* bytes, Argument::Address count)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      if (it->mappedAddr <= address && it->mappedAddr + it->size > address)
      {
        Argument::Address offset = address - it->mappedAddr;
        Argument::Address toCopy = std::min(count, it->size - offset);
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

  bool ReadBytes(Argument::Address address, Argument::Data* bytes, Argument::Address count)
  {
    MemoryMap::iterator it = memoryMap.begin();
    while (it != memoryMap.end())
    {
      if (it->mappedAddr <= address && it->mappedAddr + it->size > address)
      {
        Argument::Address offset = address - it->mappedAddr;
        Argument::Address toCopy = std::min(count, it->size - offset);
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

  bool WriteString(Argument::Address address, const std::string& string)
  {
    return WriteBytes(address, (Argument::Data*)(string.data()), Argument::Address(string.length() + 1));
  }

  bool ReadString(Argument::Address address, std::string& string)
  {
    string.clear();
    Argument::Data c = '\0';
    do
    {
      if (!ReadByte(address++, c))
        return false;
      string += char(c);
    } while (c);
    return true;
  }

  bool SetStackPointer(Argument::Address address)
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

  bool GetStackPointer(Argument::Address& address)
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
