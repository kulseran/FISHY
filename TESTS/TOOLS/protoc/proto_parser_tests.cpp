#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/types.h>
#include <TOOLS/protoc/proto_parser.h>

using core::types::ProtoDef;

REGISTER_TEST_CASE(parseEmpty) {
  ProtoDef out;
  const std::string testFile = "";
  TEST(testing::assertFalse(proto::parse(out, testFile)));
}

REGISTER_TEST_CASE(parsePackage) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertEquals("foo.bar.baz", out.m_package));
}

REGISTER_TEST_CASE(parseImport) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "import \"CORE/NET/settings.proto\";";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertEquals(1, out.m_imports.size()));
  TEST(testing::assertEquals("CORE/NET/settings.proto", out.m_imports.at(0)));
}

REGISTER_TEST_CASE(parseEmptyMessage) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message testmessage {\n"
                               "}\n";
  TEST(testing::assertFalse(proto::parse(out, testFile)));
}

REGISTER_TEST_CASE(parseMessageWithFields) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message testmessage {\n"
                               "  double field01 = 1;\n"
                               "  float field02 = 2;\n"
                               "  int32 field03 = 3;\n"
                               "  int64 field04 = 4;\n"
                               "  uint32 field05 = 5;\n"
                               "  uint64 field06 = 6;\n"
                               "  sint32 field07 = 7;\n"
                               "  sint64 field08 = 8;\n"
                               "  fixed32 field09 = 9;\n"
                               "  fixed64 field10 = 10;\n"
                               "  sfixed32 field11 = 11;\n"
                               "  sfixed64 field12 = 12;\n"
                               "  bool field13 = 13;\n"
                               "  string field14 = 14;\n"
                               "  bytes field15 = 15;\n"
                               "  fake.name field16[] = 16;\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertEquals(1, out.m_messages.size()));
  TEST(testing::assertEquals(16, out.m_messages.at(0).m_fields.size()));

  TEST(testing::assertEquals(
      "field01", out.m_messages.at(0).m_fields.at(0).m_name));
  TEST(
      testing::assertEquals("", out.m_messages.at(0).m_fields.at(0).m_msgType));
  TEST(
      testing::assertEquals(1, out.m_messages.at(0).m_fields.at(0).m_fieldNum));
  TEST(testing::assertEquals(
      false, out.m_messages.at(0).m_fields.at(0).m_repeated));

  TEST(testing::assertEquals(
      "field16", out.m_messages.at(0).m_fields.at(15).m_name));
  TEST(testing::assertEquals(
      "fake.name", out.m_messages.at(0).m_fields.at(15).m_msgType));
  TEST(testing::assertEquals(
      16, out.m_messages.at(0).m_fields.at(15).m_fieldNum));
  TEST(testing::assertEquals(
      true, out.m_messages.at(0).m_fields.at(15).m_repeated));
}

REGISTER_TEST_CASE(parseExtraPackageFail) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz; package foo.bar.bar;";
  TEST(testing::assertFalse(proto::parse(out, testFile)));
}

REGISTER_TEST_CASE(parseToplevelEnumFails) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "enum testenum {\n"
                               "}\n";
  TEST(testing::assertFalse(proto::parse(out, testFile)));
}

REGISTER_TEST_CASE(parseMessageWithNestedEnum) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message testmessage {\n"
                               "  enum testenum {\n"
                               "    VALUE_1 = 0;\n"
                               "    VALUE_2 = 1;\n"
                               "  }\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertEquals(1, out.m_messages.size()));
  TEST(testing::assertEquals(1, out.m_messages.at(0).m_enums.size()));
  TEST(testing::assertEquals(
      2, out.m_messages.at(0).m_enums.at(0).m_values.size()));
}

REGISTER_TEST_CASE(parseMessageWithNestedEnums) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message testmessage {\n"
                               "  enum testenum {\n"
                               "    VALUE_1 = 0;\n"
                               "    VALUE_2 = 1;\n"
                               "  }\n"
                               "  enum testenum2 {\n"
                               "    VALUE_3 = 0;\n"
                               "    VALUE_4 = 1;\n"
                               "  }\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertEquals(1, out.m_messages.size()));
  TEST(testing::assertEquals(2, out.m_messages.at(0).m_enums.size()));
  TEST(testing::assertEquals(
      2, out.m_messages.at(0).m_enums.at(0).m_values.size()));
}

REGISTER_TEST_CASE(parseMessageWithNestedMessage) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message testmessage {\n"
                               "  message testmessage2 {\n"
                               "    int32 field1 = 1;\n"
                               "  }\n"
                               "  int32 field1 = 1;\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertEquals(1, out.m_messages.size()));
  TEST(testing::assertEquals(1, out.m_messages.at(0).m_fields.size()));
  TEST(testing::assertEquals(1, out.m_messages.at(0).m_messages.size()));
  TEST(testing::assertEquals(
      1, out.m_messages.at(0).m_messages.at(0).m_fields.size()));
}

REGISTER_TEST_CASE(parseMessageWithNestedMessage2) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "message testmessage {\n"
                               "  int32 field1 = 1;\n"
                               "  message testmessage2 {\n"
                               "    int32 field1 = 1;\n"
                               "  }\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertEquals(1, out.m_messages.size()));
  TEST(testing::assertEquals(1, out.m_messages.at(0).m_fields.size()));
  TEST(testing::assertEquals(1, out.m_messages.at(0).m_messages.size()));
  TEST(testing::assertEquals(
      1, out.m_messages.at(0).m_messages.at(0).m_fields.size()));
}

REGISTER_TEST_CASE(parseService) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "service TestService {\n"
                               "  rpc Foobar(Bar) returns (Baz);\n"
                               "  rpc Foobaz(Baz) returns (Bar);\n"
                               "}\n";
  TEST(testing::assertTrue(proto::parse(out, testFile)));
  TEST(testing::assertEquals(1, out.m_services.size()));
  TEST(testing::assertEquals("TestService", out.m_services.at(0).m_name));
  TEST(testing::assertEquals(2, out.m_services.at(0).m_functions.size()));
  TEST(testing::assertEquals(
      "Foobar", out.m_services.at(0).m_functions.at(0).m_name));
  TEST(testing::assertEquals(
      "Bar", out.m_services.at(0).m_functions.at(0).m_param));
  TEST(testing::assertEquals(
      "Baz", out.m_services.at(0).m_functions.at(0).m_return));
}

REGISTER_TEST_CASE(parseErrorMissingRpc) {
  ProtoDef out;
  const std::string testFile = "package foo.bar.baz;\n"
                               "service TestService {\n"
                               "}\n";
  TEST(testing::assertFalse(proto::parse(out, testFile)));
}

REGISTER_TEST_CASE(parseErrorMissingCloseingBrace) {
  ProtoDef out;
  const std::string testFile = "message testmessage {\n";
  TEST(testing::assertFalse(proto::parse(out, testFile)));
}

REGISTER_TEST_CASE(parseErrorMissingMessageName) {
  ProtoDef out;
  const std::string testFile = "message {\n"
                               "}\n";
  TEST(testing::assertFalse(proto::parse(out, testFile)));
}

REGISTER_TEST_CASE(parseErrorMissingFieldSpecifier) {
  ProtoDef out;
  const std::string testFile = "message testmessage {\n"
                               "  uint32 field = 5;\n"
                               "}\n";
  TEST(testing::assertFalse(proto::parse(out, testFile)));
}

REGISTER_TEST_CASE(parseErrorMissingFieldType) {
  ProtoDef out;
  const std::string testFile = "message testmessage {\n"
                               "  field = 5;\n"
                               "}\n";
  TEST(testing::assertFalse(proto::parse(out, testFile)));
}

REGISTER_TEST_CASE(parseErrorMissingFieldNum) {
  ProtoDef out;
  const std::string testFile = "message testmessage {\n"
                               "  uint32 field;\n"
                               "}\n";
  TEST(testing::assertFalse(proto::parse(out, testFile)));
}

REGISTER_TEST_CASE(parseErrorMissingFieldEol) {
  ProtoDef out;
  const std::string testFile = "message testmessage {\n"
                               "  uint32 field = 5\n"
                               "}\n";
  TEST(testing::assertFalse(proto::parse(out, testFile)));
}
