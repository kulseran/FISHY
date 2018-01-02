#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/FILES/proto_text.h>

#include <TESTS/CORE/UTIL/FILES/textproto_testproto.pb.h>

using core::util::files::TextFormat;
using test::core::util::files::textproto::TestProto;
using test::core::util::files::textproto::TestProtoEmbeded;

REGISTER_TEST_CASE(testTextFormatUnformat) {
  std::string out;
  TestProto testProto =
      TestProto::Builder()
          .add_a_string("hello")
          .add_a_string("world")
          .set_a_message(
              TestProtoEmbeded::Builder().set_a_string("good\nbye").build())
          .build();
  TextFormat::format(out, testProto);

  TestProto result;
  TEST(testing::assertEquals(
      TextFormat::parse(result, out).getStatus(), Status::OK));
  TEST(testing::assertEquals(testProto, result));
}
