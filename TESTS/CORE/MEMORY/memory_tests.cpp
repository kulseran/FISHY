#include <TESTS/testcase.h>
#include <TESTS/test_assertions.h>

#include <CORE/types.h>
#include <CORE/MEMORY/memory.h>

using core::memory::isPow2;
using core::memory::nextPow2;
using core::memory::allocAligned;
using core::memory::freeAligned;

REGISTER_TEST_CASE(testPow2) {
  TEST(testing::assertTrue(isPow2((intptr_t) 1)));
  TEST(testing::assertTrue(isPow2((intptr_t) 2)));
  TEST(testing::assertFalse(isPow2((intptr_t) 3)));
  TEST(testing::assertTrue(isPow2((intptr_t) 4)));
  TEST(testing::assertFalse(isPow2((intptr_t) 5)));
  TEST(testing::assertFalse(isPow2((intptr_t) 6)));
  TEST(testing::assertFalse(isPow2((intptr_t) 7)));
  TEST(testing::assertTrue(isPow2((intptr_t) 8)));
  TEST(testing::assertTrue(isPow2((intptr_t) 16)));
  TEST(testing::assertTrue(isPow2((intptr_t) 32)));
  TEST(testing::assertTrue(isPow2((intptr_t) 64)));
}

REGISTER_TEST_CASE(testNextPow2) {
  TEST(testing::assertEquals(nextPow2(0), 1));
  TEST(testing::assertEquals(nextPow2(1), 1));
  TEST(testing::assertEquals(nextPow2(2), 2));
  TEST(testing::assertEquals(nextPow2(3), 4));
  TEST(testing::assertEquals(nextPow2(5), 8));
  TEST(testing::assertEquals(nextPow2(15), 16));
  TEST(testing::assertEquals(nextPow2(120), 128));
}

REGISTER_TEST_CASE(testAllocFree) {
  u8 *testPtr = (u8 *) allocAligned(16, 16);
  TEST(testing::assertEquals(int ((intptr_t) testPtr & 15), 0));
  freeAligned(testPtr);
}

REGISTER_TEST_CASE(testReFree) {
  freeAligned(nullptr);
}
