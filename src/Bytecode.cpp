#include "pch.h"
#include <ctime>
#include "kipBytecode.h"
#include "kipVersion.h"

namespace kip
{
namespace Bytecode
{
  Metadata::Metadata()
  {
  }

  const uint8_t* Metadata::data() const
  {
    return rawData.data();
  }

  Metadata::size_type Metadata::size() const
  {
    return rawData.size();
  }

  Metadata Metadata::Comment(const std::string& comment)
  {
    Metadata block;
    block.rawData.resize(comment.size() + 2);
    block.rawData[0] = uint8_t(Type::COMMENT);
    block.rawData.back() = 0;
    std::copy(comment.begin(), comment.end(), block.rawData.begin() + 1);
    return block;
  }

  Metadata Metadata::Author(const std::string& author)
  {
    Metadata block;
    block.rawData.resize(author.size() + 2);
    block.rawData[0] = uint8_t(Type::AUTHOR);
    block.rawData.back() = 0;
    std::copy(author.begin(), author.end(), block.rawData.begin() + 1);
    return block;
  }

  Metadata Metadata::Timestamp()
  {
    struct tm* currentTime = std::localtime(NULL);
    return Timestamp(currentTime->tm_year + 1900, currentTime->tm_mon, currentTime->tm_mday);
  }

  Metadata Metadata::Timestamp(uint16_t year, uint8_t month, uint8_t day)
  {
    struct tm* currentTime = localtime(NULL);
    return Timestamp(year, month, day, currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);
  }

  Metadata Metadata::Timestamp(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
  {
    Metadata block;
    block.rawData.resize(8);
    block.rawData[0] = uint8_t(Type::TIMESTAMP);
    block.rawData[1] = year >> 8;
    block.rawData[2] = year % 0xFF;
    block.rawData[3] = month;
    block.rawData[4] = day;
    block.rawData[5] = hour;
    block.rawData[6] = minute;
    block.rawData[7] = second;
    return block;
  }

/////////////////////////////////////////////////////////////////////////////////////////////////

  Header::Header()
    : Header(versionMajor, versionMinor)
  {
  }

  Header::Header(uint8_t versionMajor, uint8_t versionMinor)
  {
    rawData[0] = 'K';
    rawData[1] = 'I';
    rawData[2] = 'P';
    rawData[3] = 0x00;
    rawData[4] = versionMajor;
    rawData[5] = versionMinor;
    for (uint8_t i = uint8_t(Offset::RESERVED_START); i <= uint8_t(Offset::RESERVED_END); ++i)
      rawData[i] = 0x00;
    for (uint8_t i = uint8_t(Offset::METADATA_BLOCK_COUNT); i < uint8_t(Offset::HEADER_END); ++i)
      rawData[i] = 0x00;
  }

  Header::Header(const std::vector<Metadata>& metadata)
    : Header()
  {
    uint32_t count = uint32_t(metadata.size());
    *(uint32_t*)(&rawData[uint8_t(Offset::METADATA_BLOCK_COUNT)]) = count;
  }

  Header::Header(const std::vector<Metadata>& metadata, uint8_t versionMajor, uint8_t versionMinor)
    : Header(versionMajor, versionMinor)
  {
    uint32_t count = uint32_t(metadata.size());
    *(uint32_t*)(&rawData[uint8_t(Offset::METADATA_BLOCK_COUNT)]) = count;
  }

  const uint8_t* Header::data() const
  {
    return rawData.data();
  }

  Header::size_type Header::size() const
  {
    return Header::size_type(Offset::HEADER_END);
  }

  Header::data_type::iterator Header::begin()
  {
    return rawData.begin();
  }

  Header::data_type::iterator Header::end()
  {
    return rawData.end();
  }
}
}
