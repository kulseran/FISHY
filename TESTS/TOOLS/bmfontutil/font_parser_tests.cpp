#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/VFS/vfs_file.h>
#include <CORE/types.h>
#include <TOOLS/bmfontutil/font_parser.h>

using bmfont::FontDef;
using core::util::files::XmlNode;

REGISTER_TEST_CASE(parseTestFont) {
  const std::string fileroot = "TESTS/TOOLS/bmfontutil/testdata/";
  vfs::ifstream ifile(vfs::Path(fileroot + "testfont.fnt"));

  TEST(testing::assertTrue(ifile.is_open()));

  std::string fileContent;
  std::copy(
      std::istreambuf_iterator< char >(ifile.rdbuf()),
      std::istreambuf_iterator< char >(),
      std::back_inserter(fileContent));

  XmlNode document;
  TEST(testing::assertTrue(document.parse(fileContent)));

  size_t fontNodeIndex;
  TEST(testing::assertTrue(document.getChild("font", fontNodeIndex)));

  FontDef out;
  TEST(testing::assertTrue(ParseDocument(
      out, vfs::Path(fileroot), document.getChild(fontNodeIndex), false)));

  TEST(testing::assertTrue(out.has_common()));
  TEST(testing::assertTrue(out.has_info()));
  TEST(testing::assertEquals(110, out.get_char_info_size()));
  TEST(testing::assertEquals(1, out.get_texture_size()));

  TEST(testing::assertEquals("Consolas", out.get_info().get_font_name()));
}

REGISTER_TEST_CASE(parseTestDistanceFont) {
  const std::string fileroot = "TESTS/TOOLS/bmfontutil/testdata/";
  vfs::ifstream ifile(vfs::Path(fileroot + "testfont.fnt"));
  TEST(testing::assertTrue(ifile.is_open()));

  std::string fileContent;
  std::copy(
      std::istreambuf_iterator< char >(ifile.rdbuf()),
      std::istreambuf_iterator< char >(),
      std::back_inserter(fileContent));

  XmlNode document;
  TEST(testing::assertTrue(document.parse(fileContent)));

  size_t fontNodeIndex;
  TEST(testing::assertTrue(document.getChild("font", fontNodeIndex)));

  FontDef out;
  TEST(testing::assertTrue(ParseDocument(
      out, vfs::Path(fileroot), document.getChild(fontNodeIndex), true)));

  TEST(testing::assertTrue(out.has_common()));
  TEST(testing::assertTrue(out.has_info()));
  TEST(testing::assertEquals(110, out.get_char_info_size()));
  TEST(testing::assertEquals(1, out.get_texture_size()));

  TEST(testing::assertEquals("Consolas", out.get_info().get_font_name()));
}
