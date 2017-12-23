#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/BASE/serializer_strings.h>

using core::base::BlobSink;
using core::memory::Blob;

REGISTER_TEST_CASE(testString) {
  const std::string expected = "hello world";

  std::string actual;
  actual.resize(1024);
  Blob actualBuf(actual);
  BlobSink sink(actualBuf);

  sink << expected;
  sink.reset();
  sink >> actual;
  TEST(testing::assertEquals(
      memcmp(actual.data(), expected.data(), expected.size()), 0));
}
