#include <TESTS/testcase.h>
#include <TESTS/test_assertions.h>

#include <CORE/types.h>
#include <CORE/HASH/crc32.h>

using namespace core::hash;

REGISTER_TEST_CASE(crc32TestHash) {
  TEST(testing::assertEquals(0x4A17B156, CRC32("Hello World", 11)));
  TEST(testing::assertEquals(0x87E5865B, CiCRC32("Hello World", 11)));
}
