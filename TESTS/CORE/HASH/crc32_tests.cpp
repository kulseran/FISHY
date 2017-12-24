#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/HASH/crc32.h>
#include <CORE/types.h>

#include <cstring>

using namespace core::hash;

REGISTER_TEST_CASE(crc32TestHash) {
  const char *hellostr = "Hello World";
  const size_t hellolen = strlen(hellostr);
  TEST(testing::assertEquals(0x4A17B156, CRC32(hellostr, hellolen)));
  TEST(testing::assertEquals(0x87E5865B, CiCRC32(hellostr, hellolen)));
  TEST(testing::assertEquals(0x4A17B156, CRC32(hellostr, hellostr + hellolen)));
  TEST(testing::assertEquals(
      0x87E5865B, CiCRC32(hellostr, hellostr + hellolen)));
}
