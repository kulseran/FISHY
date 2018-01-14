#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/FILES/xml_parser.h>
#include <CORE/VFS/vfs_file.h>

using core::util::files::XmlNode;

REGISTER_TEST_CASE(testParseTest) {
  vfs::ifstream ifile(
      vfs::Path("TESTS/CORE/UTIL/FILES/testdata/xml_parser_tests.xml"));
  TEST(testing::assertTrue(ifile.is_open()));

  std::string fileContent;
  std::copy(
      std::istreambuf_iterator< char >(ifile.rdbuf()),
      std::istreambuf_iterator< char >(),
      std::back_inserter(fileContent));

  XmlNode node;
  bool ret = node.parse(fileContent);
  TEST(testing::assertTrue(ret));

  size_t index;
  ret = node.getChild("chars", index);
  TEST(testing::assertTrue(ret));
  TEST(testing::assertEquals(19, node.getChild(index).getChildCount()));

  ret = node.getChild("info", index);
  TEST(testing::assertTrue(ret));

  std::string val;
  ret = node.getChild(index).getAttribute("spacing", val);
  TEST(testing::assertTrue(ret));
  TEST(testing::assertEquals("1,1", val));
}
