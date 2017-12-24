#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/types.h>
#include <TOOLS/protoc/proto_parser.h>
#include <TOOLS/protoc/proto_validator.h>

using core::types::ProtoDef;

REGISTER_TEST_CASE(validateEmpty) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertTrue(verifyDef(out)));
}

REGISTER_TEST_CASE(validateErrorRepeateMessage) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message foo { int32 field = 1; }\n"
                               "message foo { int32 field = 1; }\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertFalse(verifyDef(out)));
}

REGISTER_TEST_CASE(validateErrorRepeateSubMessage) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message foo { \n"
                               "  message bar { int32 field = 1; }\n"
                               "  message bar { int32 field = 1; }\n"
                               "  int32 field = 1;\n"
                               "}";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertFalse(verifyDef(out)));
}

REGISTER_TEST_CASE(validateErrorRepeateServiceFunctions) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "service TestService {\n"
                               "  rpc Foobar(Bar) returns (Baz);\n"
                               "  rpc Foobar(Baz) returns (Bar);\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertFalse(verifyDef(out)));
}

REGISTER_TEST_CASE(validateErrorRepeateServices) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "service TestService {\n"
                               "  rpc Foobar(Bar) returns (Baz);\n"
                               "}\n"
                               "service TestService {\n"
                               "  rpc Foobar(Bar) returns (Baz);\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertFalse(verifyDef(out)));
}

REGISTER_TEST_CASE(validateErrorRepeateMessageServices) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message TestService {\n"
                               "  int32 field = 1;\n"
                               "}\n"
                               "service TestService {\n"
                               "  rpc Foobar(Bar) returns (Baz);\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertFalse(verifyDef(out)));
}

REGISTER_TEST_CASE(validateErrorInvalidFieldNum) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message foo {\n"
                               "  int32 field = 0;\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertFalse(verifyDef(out)));
}

REGISTER_TEST_CASE(validateErrorRepeatedFieldName) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message foo {\n"
                               "  int32 field = 1;\n"
                               "  int32 field = 2;\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertFalse(verifyDef(out)));
}

REGISTER_TEST_CASE(validateErrorRepeatedFieldNum) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message foo {\n"
                               "  int32 field1 = 1;\n"
                               "  int32 field2 = 1;\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertFalse(verifyDef(out)));
}

REGISTER_TEST_CASE(validateErrorInvalidEnumNum) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message foo {\n"
                               "  enum bar {\n"
                               "    value = 0;"
                               "  }\n"
                               "  int32 field = 1;\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertFalse(verifyDef(out)));
}

REGISTER_TEST_CASE(validateErrorRepeatedEnumName) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message foo {\n"
                               "  enum bar {\n"
                               "    value = 1;"
                               "    value = 2;"
                               "  }\n"
                               "  int32 field = 1;\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertFalse(verifyDef(out)));
}

REGISTER_TEST_CASE(validateErrorRepeatedEnumNum) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message foo {\n"
                               "  enum bar {\n"
                               "    value1 = 1;"
                               "    value2 = 1;"
                               "  }\n"
                               "  int32 field = 1;\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertFalse(verifyDef(out)));
}
