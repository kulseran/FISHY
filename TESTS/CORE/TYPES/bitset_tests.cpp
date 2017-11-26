#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/TYPES/bitset.h>
#include <CORE/types.h>

using core::types::BitSet;

struct eTestBits8 {
  enum type { COUNT = 8 };
};

struct eTestBits16 {
  enum type {
    BIT0,
    BIT1,
    BIT2,
    BIT3,
    BIT4,
    BIT5,
    BIT6,
    BIT7,
    BIT8,
    BIT9,
    BIT10,
    BIT11,
    BIT12,
    BIT13,
    BIT14,
    BIT15,
    COUNT
  };
};

struct eTestBits32 {
  enum type { COUNT = 32 };
};

struct eTestBits512 {
  enum type { COUNT = 512 };
};

REGISTER_TEST_CASE(testSizes) {
  TEST(testing::assertEquals(1, sizeof(BitSet< eTestBits8 >)));
  TEST(testing::assertEquals(2, sizeof(BitSet< eTestBits16 >)));
  TEST(testing::assertEquals(4, sizeof(BitSet< eTestBits32 >)));
  TEST(testing::assertEquals(64, sizeof(BitSet< eTestBits512 >)));
}

REGISTER_TEST_CASE(testSet) {
  BitSet< eTestBits16 > bits;

  TEST(testing::assertFalse(bits.isSet(eTestBits16::BIT0)));
  TEST(testing::assertFalse(bits.isSet(eTestBits16::BIT6)));
  TEST(testing::assertFalse(bits.isSet(eTestBits16::BIT15)));

  bits.set(eTestBits16::BIT0);
  bits.set(eTestBits16::BIT15);

  TEST(testing::assertTrue(bits.isSet(eTestBits16::BIT0)));
  TEST(testing::assertFalse(bits.isSet(eTestBits16::BIT6)));
  TEST(testing::assertTrue(bits.isSet(eTestBits16::BIT15)));
}

REGISTER_TEST_CASE(testUnSet) {
  BitSet< eTestBits16 > bits;

  bits.set(eTestBits16::BIT0);
  bits.set(eTestBits16::BIT15);
  TEST(testing::assertTrue(bits.isSet(eTestBits16::BIT0)));
  TEST(testing::assertFalse(bits.isSet(eTestBits16::BIT6)));
  TEST(testing::assertTrue(bits.isSet(eTestBits16::BIT15)));

  bits.unset(eTestBits16::BIT15);
  TEST(testing::assertTrue(bits.isSet(eTestBits16::BIT0)));
  TEST(testing::assertFalse(bits.isSet(eTestBits16::BIT6)));
  TEST(testing::assertFalse(bits.isSet(eTestBits16::BIT15)));
}

REGISTER_TEST_CASE(testLogicalAnd) {
  BitSet< eTestBits16 > bits1;
  BitSet< eTestBits16 > bits2;

  bits1.set(eTestBits16::BIT0);
  bits2.set(eTestBits16::BIT0);
  bits1.set(eTestBits16::BIT11);

  // { 0, 11 } & { 0 } -> { 0 }
  BitSet< eTestBits16 > result1 = bits1 & bits2;
  TEST(testing::assertTrue(result1.isSet(eTestBits16::BIT0)));
  TEST(testing::assertFalse(result1.isSet(eTestBits16::BIT11)));

  // { 0, 11 } & { 11 } -> { 11 }
  bool result2 = bits1 & eTestBits16::BIT11;
  TEST(testing::assertTrue(result2));

  // { 0, 11 } & { 10 } -> { }
  bool result3 = bits1 & eTestBits16::BIT10;
  TEST(testing::assertFalse(result3));
}

REGISTER_TEST_CASE(testLogicalOr) {
  BitSet< eTestBits16 > bits1;
  BitSet< eTestBits16 > bits2;
  BitSet< eTestBits16 > bits3;

  bits1.set(eTestBits16::BIT0);
  bits2.set(eTestBits16::BIT0);
  bits1.set(eTestBits16::BIT11);
  bits3.set(eTestBits16::BIT10);

  // { 0, 11 } | { 0 } -> { 0, 11 }
  BitSet< eTestBits16 > result1 = bits1 | bits2;
  TEST(testing::assertTrue(result1.isSet(eTestBits16::BIT0)));
  TEST(testing::assertTrue(result1.isSet(eTestBits16::BIT11)));

  // { 0, 11 } | { 10 } -> { 0, 10, 11 }
  BitSet< eTestBits16 > result2 = bits1 | bits3;
  TEST(testing::assertTrue(result2.isSet(eTestBits16::BIT0)));
  TEST(testing::assertTrue(result2.isSet(eTestBits16::BIT10)));
  TEST(testing::assertTrue(result2.isSet(eTestBits16::BIT11)));

  // { 0, 11 } | { 11 } -> { 0, 11 }
  BitSet< eTestBits16 > result3 = bits1 | eTestBits16::BIT11;
  TEST(testing::assertTrue(result3.isSet(eTestBits16::BIT0)));
  TEST(testing::assertTrue(result3.isSet(eTestBits16::BIT11)));

  // { 0, 11 } | { 101 } -> { 0, 10, 11 }
  BitSet< eTestBits16 > result4 = bits1 | eTestBits16::BIT10;
  TEST(testing::assertTrue(result4.isSet(eTestBits16::BIT0)));
  TEST(testing::assertTrue(result4.isSet(eTestBits16::BIT10)));
  TEST(testing::assertTrue(result4.isSet(eTestBits16::BIT11)));
}
