#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/BASE/serializer_streamsink.h>
#include <CORE/BASE/serializer_strings.h>

#include <strstream>

using core::base::InStreamSink;
using core::base::OutStreamSink;

REGISTER_TEST_CASE(testStreamSink) {
  const std::string expected = "hello world";
  std::string actual;
  std::stringstream buffer;
  InStreamSink inSink(buffer);
  OutStreamSink outSink(buffer);
  outSink << expected;
  inSink >> actual;
  TEST(testing::assertEquals(
      0, memcmp(actual.data(), expected.data(), expected.size())));
}
