#include "font_parser.h"
#include "distancefield.h"

#include <CORE/BASE/config.h>
#include <CORE/BASE/logging.h>
#include <CORE/UTIL/FILES/tga_parser.h>
#include <CORE/UTIL/stringutil.h>
#include <CORE/VFS/vfs_file.h>

using bmfont::FontDef;
using core::memory::Blob;
using core::memory::ConstBlob;
using core::util::lexical_cast;
using core::util::files::XmlNode;

static bool convertImageToDistanceField(std::string &);
static Status ParseInfo(FontDef::FontInfo &out, const XmlNode &document);
static Status
ParseCommonData(FontDef::FontCommonData &out, const XmlNode &document);
static Status ParseChars(FontDef::Builder &out, const XmlNode &document);
static Status ParsePages(
    FontDef::Builder &out,
    const vfs::Path &rootPath,
    const XmlNode &document,
    const bool asDistanceFont);
static Status ConvertImageToDistanceField(std::string &imageBytes);

core::config::Flag< int > g_distanceScale(
    "distance_scale", "scales the distance range in pixels", 64);

/**
 *
 */
Status ParseDocument(
    FontDef &out,
    const vfs::Path &rootPath,
    const XmlNode &document,
    const bool asDistanceFont) {
  size_t nodeIndex;

  FontDef::Builder defBuilder;

  // Info
  {
    RET_SM(
        document.getChild("info", nodeIndex),
        Status::BAD_INPUT,
        "Document missing required 'info' node.");
    const XmlNode &infoNode = document.getChild(nodeIndex);
    FontDef::FontInfo fontInfo;
    Status ret = ParseInfo(fontInfo, infoNode);
    RET_SM(ret, ret.clone(), "Unable to parse FontInfo.");
    defBuilder.set_info(fontInfo);
  }

  // Common
  {
    RET_SM(
        document.getChild("common", nodeIndex),
        Status::BAD_INPUT,
        "Document missing required 'common' node.");
    const XmlNode &commonNode = document.getChild(nodeIndex);
    FontDef::FontCommonData commonData;
    Status ret = ParseCommonData(commonData, commonNode);
    RET_SM(ret, ret.clone(), "Unable to parse FontCommonData.");
    defBuilder.set_common(commonData);
  }

  // Chars
  {
    RET_SM(
        document.getChild("chars", nodeIndex),
        Status::BAD_INPUT,
        "Document missing required 'chars' node.");
    const XmlNode &charsNode = document.getChild(nodeIndex);
    Status ret = ParseChars(defBuilder, charsNode);
    RET_SM(ret, ret.clone(), "Unable to parse character set.");
  }

  // Texture Pages
  {
    RET_SM(
        document.getChild("pages", nodeIndex),
        Status::BAD_INPUT,
        "Document missing required 'pages' node.");
    const XmlNode &pagesNode = document.getChild(nodeIndex);
    FontDef::FontCommonData commonData;
    Status ret = ParsePages(defBuilder, rootPath, pagesNode, asDistanceFont);
    RET_SM(ret, ret.clone(), "Unable to parse texture set.");
  }

  out = defBuilder.build();
  return Status::OK;
}

/**
 *
 */
Status ParseInfo(FontDef::FontInfo &out, const XmlNode &document) {
  FontDef::FontInfo::Builder infoBuilder;

  std::string value;

  // charset
  RET_SM(
      document.getAttribute("charset", value),
      Status::BAD_INPUT,
      "Missing attribute 'charset' while parsing FontInfo.");
  if (value == "ANSI") {
    infoBuilder.set_char_set(0);
  } else {
    infoBuilder.set_char_set(-1);
  }

  // Misc
  {
    RET_SM(
        document.getAttribute("aa", value),
        Status::BAD_INPUT,
        "Missing attribute 'aa' while parsing FontInfo.");
    u32 aa;
    CHECK(lexical_cast(value, aa));
    infoBuilder.set_aa(aa != 0);
  }
  {
    RET_SM(
        document.getAttribute("bold", value),
        Status::BAD_INPUT,
        "Missing attribute 'aa' while parsing FontInfo.");
    u32 bold;
    CHECK(lexical_cast(value, bold));
    infoBuilder.set_bold(bold != 0);
  }
  {
    RET_SM(
        document.getAttribute("face", value),
        Status::BAD_INPUT,
        "Missing attribute 'aa' while parsing FontInfo.");
    infoBuilder.set_font_name(value);
  }
  {
    RET_SM(
        document.getAttribute("size", value),
        Status::BAD_INPUT,
        "Missing attribute 'aa' while parsing FontInfo.");
    f32 size;
    CHECK(lexical_cast(value, size));
    infoBuilder.set_font_size(size);
  }
  {
    RET_SM(
        document.getAttribute("italic", value),
        Status::BAD_INPUT,
        "Missing attribute 'aa' while parsing FontInfo.");
    u32 italic;
    CHECK(lexical_cast(value, italic));
    infoBuilder.set_italic(italic != 0);
  }
  {
    RET_SM(
        document.getAttribute("outline", value),
        Status::BAD_INPUT,
        "Missing attribute 'aa' while parsing FontInfo.");
    u32 outline;
    CHECK(lexical_cast(value, outline));
    infoBuilder.set_outline(outline);
  }
  {
    RET_SM(
        document.getAttribute("stretchH", value),
        Status::BAD_INPUT,
        "Missing attribute 'aa' while parsing FontInfo.");
    u32 stretchH;
    CHECK(lexical_cast(value, stretchH));
    infoBuilder.set_stretch_h(stretchH);
  }
  {
    RET_SM(
        document.getAttribute("unicode", value),
        Status::BAD_INPUT,
        "Missing attribute 'aa' while parsing FontInfo.");
    u32 unicode;
    CHECK(lexical_cast(value, unicode));
    infoBuilder.set_unicode(unicode != 0);
  }

  // padding
  {
    RET_SM(
        document.getAttribute("padding", value),
        Status::BAD_INPUT,
        "Missing attribute 'padding' while parsing FontInfo.");
    std::vector< std::string > splitPadding =
        core::util::Splitter().on(',').split(value);
    RET_SM(
        splitPadding.size() == 4,
        Status::BAD_INPUT,
        "Padding '" << value << "' should have 4 parts.");
    u32 pad;
    CHECK(lexical_cast(splitPadding[0], pad));
    infoBuilder.set_padding_up(pad);
    CHECK(lexical_cast(splitPadding[1], pad));
    infoBuilder.set_padding_right(pad);
    CHECK(lexical_cast(splitPadding[2], pad));
    infoBuilder.set_padding_down(pad);
    CHECK(lexical_cast(splitPadding[3], pad));
    infoBuilder.set_padding_left(pad);
  }

  // spacing
  {
    RET_SM(
        document.getAttribute("spacing", value),
        Status::BAD_INPUT,
        "Missing attribute 'spacing' while parsing FontInfo.");
    std::vector< std::string > splitSpacing =
        core::util::Splitter().on(',').split(value);
    RET_SM(
        splitSpacing.size() == 2,
        Status::BAD_INPUT,
        "Spacing '" << value << "' should have 2 parts.");
    u32 spacing;
    CHECK(lexical_cast(splitSpacing[0], spacing));
    infoBuilder.set_spacing_horiz(spacing);
    CHECK(lexical_cast(splitSpacing[1], spacing));
    infoBuilder.set_spacing_vert(spacing);
  }

  out = infoBuilder.build();
  return Status::OK;
}

/**
 *
 */
Status ParseCommonData(FontDef::FontCommonData &out, const XmlNode &document) {
  FontDef::FontCommonData::Builder commonBuilder;

  std::string value;
  {
    if (document.getAttribute("alphatest", value)) {
      u32 intval;
      CHECK(lexical_cast(value, intval));
      commonBuilder.set_alpha_test(intval != 0);
      commonBuilder.set_alpha_blend(intval == 0);
    } else {
      commonBuilder.set_alpha_test(false);
      commonBuilder.set_alpha_blend(true);
    }
  }
  {
    RET_SM(
        document.getAttribute("alphaChnl", value),
        Status::BAD_INPUT,
        "Missing attribute 'alphaChnl' while parsing FontInfo.");
    u32 intval;
    CHECK(lexical_cast(value, intval));
    commonBuilder.set_alpha_chnl(intval);
  }
  {
    RET_SM(
        document.getAttribute("blueChnl", value),
        Status::BAD_INPUT,
        "Missing attribute 'blueChnl' while parsing FontInfo.");
    u32 intval;
    CHECK(lexical_cast(value, intval));
    commonBuilder.set_blue_chnl(intval);
  }
  {
    RET_SM(
        document.getAttribute("greenChnl", value),
        Status::BAD_INPUT,
        "Missing attribute 'greenChnl' while parsing FontInfo.");
    u32 intval;
    CHECK(lexical_cast(value, intval));
    commonBuilder.set_green_chnl(intval);
  }
  {
    RET_SM(
        document.getAttribute("redChnl", value),
        Status::BAD_INPUT,
        "Missing attribute 'redChnl' while parsing FontInfo.");
    u32 intval;
    CHECK(lexical_cast(value, intval));
    commonBuilder.set_red_chnl(intval);
  }
  {
    RET_SM(
        document.getAttribute("base", value),
        Status::BAD_INPUT,
        "Missing attribute 'base' while parsing FontInfo.");
    u32 intval;
    CHECK(lexical_cast(value, intval));
    commonBuilder.set_base(intval);
  }
  {
    RET_SM(
        document.getAttribute("lineHeight", value),
        Status::BAD_INPUT,
        "Missing attribute 'lineHeight' while parsing FontInfo.");
    u32 intval;
    CHECK(lexical_cast(value, intval));
    commonBuilder.set_line_height(intval);
  }
  {
    RET_SM(
        document.getAttribute("scaleH", value),
        Status::BAD_INPUT,
        "Missing attribute 'scaleH' while parsing FontInfo.");
    u32 intval;
    CHECK(lexical_cast(value, intval));
    commonBuilder.set_scale_h(intval);
  }
  {
    RET_SM(
        document.getAttribute("scaleW", value),
        Status::BAD_INPUT,
        "Missing attribute 'scaleW' while parsing FontInfo.");
    u32 intval;
    CHECK(lexical_cast(value, intval));
    commonBuilder.set_scale_w(intval);
  }

  commonBuilder.set_bits(8);
  out = commonBuilder.build();
  return Status::OK;
}

/**
 *
 */
Status ParseChars(FontDef::Builder &out, const XmlNode &document) {
  {
    std::string value;
    RET_SM(
        document.getAttribute("count", value),
        Status::BAD_INPUT,
        "Missing attribute 'count' while parsing charInfoNode.");
    u32 count;
    CHECK(lexical_cast(value, count));
    RET_SM(
        count == document.getChildCount(),
        Status::BAD_INPUT,
        "Missing character sets. Expected: " << count << " Found: "
                                             << document.getChildCount());
  }
  for (u32 index = 0; index < document.getChildCount(); ++index) {
    const XmlNode &charInfoNode = document.getChild(index);
    FontDef::FontCharInfo::Builder infoBuilder;

    std::string value;
    {
      RET_SM(
          charInfoNode.getAttribute("chnl", value),
          Status::BAD_INPUT,
          "Missing attribute 'chnl' while parsing FontInfo.");
      u32 intval;
      CHECK(lexical_cast(value, intval));
      infoBuilder.set_chnl(intval);
    }
    {
      RET_SM(
          charInfoNode.getAttribute("height", value),
          Status::BAD_INPUT,
          "Missing attribute 'height' while parsing FontInfo.");
      f32 floatval;
      CHECK(lexical_cast(value, floatval));
      infoBuilder.set_height(floatval);
    }
    {
      RET_SM(
          charInfoNode.getAttribute("id", value),
          Status::BAD_INPUT,
          "Missing attribute 'id' while parsing FontInfo.");
      u32 intval;
      CHECK(lexical_cast(value, intval));
      infoBuilder.set_id(intval);
    }
    {
      RET_SM(
          charInfoNode.getAttribute("page", value),
          Status::BAD_INPUT,
          "Missing attribute 'page' while parsing FontInfo.");
      u32 intval;
      CHECK(lexical_cast(value, intval));
      infoBuilder.set_page(intval);
    }
    {
      RET_SM(
          charInfoNode.getAttribute("width", value),
          Status::BAD_INPUT,
          "Missing attribute 'width' while parsing FontInfo.");
      f32 floatval;
      CHECK(lexical_cast(value, floatval));
      infoBuilder.set_width(floatval);
    }
    {
      RET_SM(
          charInfoNode.getAttribute("x", value),
          Status::BAD_INPUT,
          "Missing attribute 'x' while parsing FontInfo.");
      f32 floatval;
      CHECK(lexical_cast(value, floatval));
      infoBuilder.set_x(floatval);
    }
    {
      RET_SM(
          charInfoNode.getAttribute("xadvance", value),
          Status::BAD_INPUT,
          "Missing attribute 'xadvance' while parsing FontInfo.");
      f32 floatval;
      CHECK(lexical_cast(value, floatval));
      infoBuilder.set_xadvance(floatval);
    }
    {
      RET_SM(
          charInfoNode.getAttribute("xoffset", value),
          Status::BAD_INPUT,
          "Missing attribute 'xoffset' while parsing FontInfo.");
      f32 floatval;
      CHECK(lexical_cast(value, floatval));
      infoBuilder.set_xoffset(floatval);
    }
    {
      RET_SM(
          charInfoNode.getAttribute("y", value),
          Status::BAD_INPUT,
          "Missing attribute 'y' while parsing FontInfo.");
      f32 floatval;
      CHECK(lexical_cast(value, floatval));
      infoBuilder.set_y(floatval);
    }
    {
      RET_SM(
          charInfoNode.getAttribute("yoffset", value),
          Status::BAD_INPUT,
          "Missing attribute 'yoffset' while parsing FontInfo.");
      f32 floatval;
      CHECK(lexical_cast(value, floatval));
      infoBuilder.set_yoffset(floatval);
    }

    out.add_char_info(infoBuilder.build());
  }
  return Status::OK;
}

/**
 *
 */
Status ParsePages(
    FontDef::Builder &out,
    const vfs::Path &rootPath,
    const XmlNode &document,
    const bool asDistanceFont) {
  for (u32 index = 0; index < document.getChildCount(); ++index) {
    const XmlNode &textureNode = document.getChild(index);
    std::string value;

    RET_SM(
        textureNode.getAttribute("file", value),
        Status::BAD_INPUT,
        "Missing attribute 'yoffset' while parsing FontTexture.");

    vfs::ifstream ifile((rootPath + vfs::Path(value)));
    RET_SM(
        ifile.is_open(),
        Status::NOT_FOUND,
        "Could not open texture: " << value);
    std::string imageBytes;
    imageBytes.reserve(static_cast< size_t >(ifile.getFileLen()));
    std::copy(
        std::istreambuf_iterator< char >(ifile.rdbuf()),
        std::istreambuf_iterator< char >(),
        std::back_inserter(imageBytes));

    core::util::files::TGAImage image;
    Status ret = image.loadFromMemory(ConstBlob(imageBytes));
    RET_SM(ret, ret.clone(), "Unable to parse indicated TGA image.");

    if (asDistanceFont) {
      Status ret = ConvertImageToDistanceField(imageBytes);
      if (!ret) {
        return ret.clone();
      }
      FontDef::FontTexture::Builder textureBuilder;
      textureBuilder.set_filename(value);
      textureBuilder.set_image_data(imageBytes);
      textureBuilder.set_is_distancefield(true);
      out.add_texture(textureBuilder.build());
    } else {
      FontDef::FontTexture::Builder textureBuilder;
      textureBuilder.set_filename(value);
      textureBuilder.set_image_data(imageBytes);
      textureBuilder.set_is_distancefield(false);
      out.add_texture(textureBuilder.build());
    }
  }

  return Status::OK;
}

/**
 *
 */
Status ConvertImageToDistanceField(std::string &imageBytes) {
  core::util::files::TGAImage image;
  CHECK(image.loadFromMemory(ConstBlob(imageBytes)));

  DistanceGrid converter(
      image.getHeader().m_width,
      image.getHeader().m_height,
      ((f32) g_distanceScale.get()) / 255.0f);
  converter.convertToSdf(
      &image.getDataRgba8()[0],
      &image.getMutableDataRgba8(
          image.getHeader().m_width, image.getHeader().m_height)[0]);
  Blob blob(imageBytes);
  return image.saveToMemory(blob);
}
