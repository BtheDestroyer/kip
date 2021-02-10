#include <gtest/gtest.h>
#include "kip.h"

class kipTestSTA : public testing::Test
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

TEST_F(kipTestSTA, StoreDecimalAtDecimal)
{
  // given
  const std::string instruction = "STA";
  constexpr unsigned paramA = 0x0ABCDEF;
  constexpr unsigned paramB = 100;
  const std::string command = instruction + " " + std::to_string(paramA) + " " + std::to_string(paramB);

  // when
  kip::InterpretResult result = kip::InterpretLine(command);

  // expect
  EXPECT_EQ(*((unsigned*)&memory[paramB]), paramA);
  EXPECT_TRUE(result);
}

TEST_F(kipTestSTA, StoreDecimalAtHex)
{
  // given
  const std::string instruction = "STA";
  constexpr unsigned paramA = 0x0ABCDEF;
  const std::string command = instruction + " " + std::to_string(paramA) + " $100";

  // when
  kip::InterpretResult result = kip::InterpretLine(command);

  // expect
  EXPECT_EQ(*((unsigned*)&memory[0x100]), paramA);
  EXPECT_TRUE(result);
}
