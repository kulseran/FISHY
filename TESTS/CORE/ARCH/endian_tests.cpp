#include <TESTS/testcase.h>
#include <TESTS/test_assertions.h>

#include <CORE/types.h>
#include <CORE/ARCH/endian.h>

using core::endian::little;
using core::endian::big;

REGISTER_TEST_CASE(endianTestSwap8) {
  const u8 in = 0x0F;
  u8 lout = little(in);
  u8 bout = big(in);
  TEST(testing::assertEquals(in, lout));
  TEST(testing::assertEquals(in, bout));
}

REGISTER_TEST_CASE(endianTestSwap16) {
  const u16 in = 0x0F5F;
  const u16 inSwapped = 0x5F0F;
  u16 lout = little(in);
  u16 bout = big(in);

  if (core::endian::isLittle()) {
    TEST(testing::assertEquals(in, lout));
    TEST(testing::assertEquals(inSwapped, bout));
  } else {
    TEST(testing::assertEquals(inSwapped, lout));
    TEST(testing::assertEquals(in, bout));
  }
}

REGISTER_TEST_CASE(endianTestSwap32) {
  const u32 in = 0x0F5F6B9B;
  const u32 inSwapped = 0x9B6B5F0F;
  u32 lout = little(in);
  u32 bout = big(in);

  if (core::endian::isLittle()) {
    TEST(testing::assertEquals(in, lout));
    TEST(testing::assertEquals(inSwapped, bout));
  } else {
    TEST(testing::assertEquals(inSwapped, lout));
    TEST(testing::assertEquals(in, bout));
  }
}

REGISTER_TEST_CASE(endianTestSwap64) {
  const u64 in = 0x0F5F6B9BA3A59587;
  const u64 inSwapped = 0x8795A5A39B6B5F0F;
  u64 lout = little(in);
  u64 bout = big(in);

  if (core::endian::isLittle()) {
    TEST(testing::assertEquals(in, lout));
    TEST(testing::assertEquals(inSwapped, bout));
  } else {
    TEST(testing::assertEquals(inSwapped, lout));
    TEST(testing::assertEquals(in, bout));
  }
}
