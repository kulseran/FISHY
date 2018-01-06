#include <TESTS/testcase.h>
#include <TESTS/test_assertions.h>

#include <CORE/types.h>
#include <CORE/ARCH/timer.h>

using namespace core::timeunit;

REGISTER_TEST_CASE(testTimeUnitSmallerToLargerInt) {
  TEST(testing::assertEquals(Micros.toMicros((u64) 1), 1ul));
  TEST(testing::assertEquals(Micros.toMillis((u64) 1), 0ul));
  TEST(testing::assertEquals(Micros.toSeconds((u64) 1), 0ul));
  TEST(testing::assertEquals(Micros.toMinutes((u64) 1), 0ul));
  TEST(testing::assertEquals(Micros.toHours((u64) 1), 0ul));
  TEST(testing::assertEquals(Micros.toDays((u64) 1), 0ul));
}

REGISTER_TEST_CASE(testTimeUnitSmallerToLargerFloat) {
  TEST(testing::assertEquals(Micros.toMicros((f64) 1.0), 1.0));
  TEST(testing::assertEquals(Micros.toMillis((f64) 1.0), 0.001));
  TEST(testing::assertEquals(Micros.toSeconds((f64) 1.0), 0.000001));
  TEST(testing::assertEquals(Micros.toMinutes((f64) 1.0), 0.000001 / 60.0));
  TEST(testing::assertEquals(Micros.toHours((f64) 1.0), 0.000001 / 3600.0));
  TEST(testing::assertEquals(Micros.toDays((f64) 1.0), 0.000001 / 86400.0));
}

REGISTER_TEST_CASE(testTimeUnitLargerToSmallerInt) {
  TEST(testing::assertEquals(Days.toMicros((u64) 1), 86400000000ul));
  TEST(testing::assertEquals(Days.toMillis((u64) 1), 86400000ul));
  TEST(testing::assertEquals(Days.toSeconds((u64) 1), 86400ul));
  TEST(testing::assertEquals(Days.toMinutes((u64) 1), 1440ul));
  TEST(testing::assertEquals(Days.toHours((u64) 1), 24ul));
  TEST(testing::assertEquals(Days.toDays((u64) 1), 1ul));
}

REGISTER_TEST_CASE(testTimeUnitLargerToOneSmallerInt) {
  TEST(testing::assertEquals(Millis.toMicros((u64) 1), 1000ul));
  TEST(testing::assertEquals(Seconds.toMillis((u64) 1), 1000ul));
  TEST(testing::assertEquals(Minute.toSeconds((u64) 1), 60ul));
  TEST(testing::assertEquals(Hours.toMinutes((u64) 1), 60ul));
  TEST(testing::assertEquals(Days.toHours((u64) 1), 24ul));
}
