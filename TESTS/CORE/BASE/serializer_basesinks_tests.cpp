#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/BASE/serializer.h>

using core::base::BlobSink;
using core::base::ConstBlobSink;
using core::base::RangeSink;
using core::memory::Blob;
using core::memory::ConstBlob;

REGISTER_TEST_CASE(testBlobSink) {
  const std::string sourceStr = "hello world";
  const std::string expected = "world";
  u8 buffer[1024] = {0};
  Blob bufferBlob(buffer, 1024);
  BlobSink source(bufferBlob);
  TEST(testing::assertEquals(source.avail(), 1024));
  TEST(testing::assertEquals(source.write(ConstBlob(sourceStr)), 11));
  TEST(testing::assertEquals(source.avail(), 1013));
  source.reset();
  TEST(testing::assertEquals(source.avail(), 1024));
  source.seek(6);
  TEST(testing::assertEquals(source.avail(), 1018));

  u8 actual[5] = {0};
  Blob outBufferBlob(actual, 5);
  TEST(testing::assertEquals(source.read(outBufferBlob), 5));
  TEST(testing::assertFalse(source.fail()));
  TEST(testing::assertEquals(source.avail(), 1013));
  TEST(testing::assertEquals(memcmp(actual, expected.data(), 5), 0));
}

REGISTER_TEST_CASE(testConstBlobSink) {
  const std::string sourceStr = "hello world";
  const std::string expected = "world";
  ConstBlobSink source(sourceStr);
  u8 actual[16] = {0};
  Blob bufferBlob(actual, 16);

  TEST(testing::assertEquals(source.avail(), 11));
  source.seek(6);
  TEST(testing::assertEquals(source.avail(), 5));
  TEST(testing::assertEquals(source.read(bufferBlob), 5));
  TEST(testing::assertFalse(source.fail()));
  TEST(testing::assertEquals(source.avail(), 0));
  source.reset();
  TEST(testing::assertEquals(source.avail(), 11));
  TEST(testing::assertEquals(memcmp(actual, expected.data(), 5), 0));
}

REGISTER_TEST_CASE(testRangeSink) {
  const std::string sourceStr = "hello world";
  const std::string expected = "ello";
  u8 buffer[1024] = {0};
  Blob bufferBlob(buffer, 1024);
  BlobSink initialSource(bufferBlob);
  RangeSink dest(initialSource, 5);
  RangeSink source(initialSource, 5);
  TEST(testing::assertEquals(dest.avail(), 5));
  TEST(testing::assertEquals(dest.write(ConstBlob(sourceStr)), 5));
  TEST(testing::assertEquals(dest.avail(), 0));
  initialSource.reset();
  TEST(testing::assertEquals(initialSource.avail(), 1024));

  u8 actual[5] = {0};
  Blob outBufferBlob(actual, 5);
  source.seek(1);
  TEST(testing::assertEquals(source.read(outBufferBlob), 4));
  TEST(testing::assertFalse(source.fail()));
  TEST(testing::assertEquals(source.avail(), 0));
  TEST(testing::assertEquals(memcmp(actual, expected.data(), 4), 0));
}
