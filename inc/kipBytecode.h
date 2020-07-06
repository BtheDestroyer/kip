#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "kipUniversal.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace kip
{
namespace Bytecode
{
  enum class DataType
  {
    INVALID = 0x00,
    INSTRUCTIONS_START = 0x01,
    INSTRUCTIONS_END = 0xEF,
    RESERVED = 0xF0,
    LABEL_STRING = 0xFD,
    LABEL_DATA = 0xFE,
    IMPORT = 0xFF,
  };

  class DLLMODE Metadata
  {
  public:
    typedef std::vector<uint8_t>::size_type size_type;
    const uint8_t* data() const;
    size_type size() const;

    static Metadata Comment(const std::string& comment);
    static Metadata Author(const std::string& author);
    static Metadata Timestamp();
    static Metadata Timestamp(uint16_t year, uint8_t month, uint8_t day);
    static Metadata Timestamp(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second = 0);

    enum class Type : uint8_t
    {
      INVALID,
      COMMENT,
      AUTHOR,
      TIMESTAMP,
      COUNT,
    };

  private:
    std::vector<uint8_t> rawData;
    
    Metadata();
    Metadata(Type type);
  };

  class DLLMODE Header
  {
  public:
    enum class Offset
    {
      KIP_IDENTIFIER = 0x00,
      MINOR_VERSION = 0x04,
      MAJOR_VERSION = 0x05,
      RESERVED_START = 0x06,
      RESERVED_END = 0x0F,
      METADATA_BLOCK_COUNT = 0x10,
      METADATA_BLOCK_START = 0x14,
      HEADER_END = METADATA_BLOCK_START,
    };

    typedef std::array<uint8_t, size_t(Offset::HEADER_END)> data_type;
    typedef data_type::size_type size_type;

    Header();
    Header(uint8_t versionMajor, uint8_t versionMinor = 0);
    Header(const std::vector<Metadata>& metadata);
    Header(const std::vector<Metadata>& metadata, uint8_t versionMajor, uint8_t versionMinor = 0);

    const uint8_t* data() const;
    size_type size() const;
    data_type::iterator begin();
    data_type::iterator end();

  private:
    data_type rawData;
  };
}
}

#pragma warning(pop)
