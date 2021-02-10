#include <gtest/gtest.h>
#include "kip.h"
#include <array>

class kipTestSTB : public testing::Test
{
  void SetUp() override
  {
    kip::MapMemory(memory.data(), (kip::Argument::Address)memory.size(), 0x0000);
    kip::SetStackPointer((kip::Argument::Address)memory.size() - 0x0100);
  }

  void TearDown() override
  {
    kip::UnmapMemory(memory.data());
  }

public:
  std::array<unsigned char, 0x0FFF> memory; // 4k of memory
};

TEST_F(kipTestSTB, StoreDecimalAtDecimal)
{
  // given
  const std::string instruction = "STB";
  constexpr unsigned paramA = 10;
  constexpr unsigned paramB = 100;
  const std::string command = instruction + " " + std::to_string(paramA) + " " + std::to_string(paramB);

  // when
  const kip::InterpretResult result = kip::InterpretLine(command);

  // expect
  EXPECT_EQ(memory[paramB], paramA);
  EXPECT_TRUE(result);
}

TEST_F(kipTestSTB, StoreDecimalAtHex)
{
  // given
  const std::string instruction = "STB";
  constexpr unsigned paramA = 10;
  const std::string command = instruction + " " + std::to_string(paramA) + " $0100";

  // when
  const kip::InterpretResult result = kip::InterpretLine(command);

  // expect
  EXPECT_EQ(memory[0x0100], paramA);
  EXPECT_TRUE(result);
}

TEST_F(kipTestSTB, StoreBinaryAtDecimal)
{
  // given
  const std::string instruction = "STB";
  constexpr unsigned paramB = 100;
  const std::string command = instruction + " :10 " + std::to_string(paramB);

  // when
  const kip::InterpretResult result = kip::InterpretLine(command);

  // expect
  EXPECT_EQ(memory[paramB], 0b10);
  EXPECT_TRUE(result);
}

TEST_F(kipTestSTB, StoreOctalAtDecimal)
{
  // given
  const std::string instruction = "STB";
  constexpr unsigned paramB = 100;
  const std::string command = instruction + " #10 " + std::to_string(paramB);

  // when
  const kip::InterpretResult result = kip::InterpretLine(command);

  // expect
  EXPECT_EQ(memory[paramB], 010);
  EXPECT_TRUE(result);
}

TEST_F(kipTestSTB, StoreDereferencedDecimalAtDecimal)
{
  // given
  constexpr unsigned addr1 = 100;
  constexpr unsigned addr2 = 200;
  constexpr unsigned value = 10;
  memory[addr1] = value;
  const std::string command = "STB *" + std::to_string(addr1) + " " + std::to_string(addr2);

  // when
  const kip::InterpretResult result = kip::InterpretLine(command);

  // expect
  EXPECT_EQ(memory[addr2], value);
  EXPECT_TRUE(result);
}


TEST_F(kipTestSTB, StoreDoubleDereferencedDecimalAtDecimal)
{
  // given
  constexpr unsigned addr1 = 100;
  constexpr unsigned addr2 = 200;
  constexpr unsigned addr3 = 300;
  constexpr unsigned value = 10;
  *((unsigned*)&memory[addr1]) = addr2;
  memory[addr2] = value;
  const std::string command = "STB **" + std::to_string(addr1) + " " + std::to_string(addr3);

  // when
  const kip::InterpretResult result = kip::InterpretLine(command);

  // expect
  EXPECT_EQ(memory[addr3], value);
  EXPECT_TRUE(result);
}
