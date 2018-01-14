#include "image.h"

#include <CORE/ARCH/endian.h>
#include <CORE/BASE/checks.h>
#include <CORE/BASE/logging.h>
#include <CORE/UTIL/FILES/tga_parser.h>

using core::util::files::TGAImage;

namespace fsengine {
namespace resources {

/**
 *
 */
bool Image::loadFromMemory(
    const FmtType fmt, const core::memory::ConstBlob &buffer) {
  m_szx = 0;
  m_szy = 0;
  m_RGBA8_data.clear();

  switch (fmt) {
    case FORMAT_TGA: {
      return serializeFromTga(buffer);
    }

    default: { CHECK_NOT_IMPLEMENTED(); }
  }
  return false;
}

/**
 *
 */
bool Image::saveToMemory(const FmtType fmt, core::memory::Blob &buffer) const {
  switch (fmt) {
    case FORMAT_TGA: {
      return serializeToTga(buffer);
    }

    default: { CHECK_NOT_IMPLEMENTED(); }
  }
  return false;
}

/**
 *
 */
size_t Image::getSaveSize(const FmtType fmt) const {
  switch (fmt) {
    case FORMAT_TGA: {
      return getSizeTga();
    }
    default: { CHECK_NOT_IMPLEMENTED(); }
  }
  return 0;
}

/**
 *
 */
Status Image::serializeFromTga(const core::memory::ConstBlob &buffer) {
  TGAImage image;
  Status ret = image.loadFromMemory(buffer);
  if (!ret) {
    return ret.clone();
  }
  m_szx = image.getHeader().m_width;
  m_szy = image.getHeader().m_height;
  m_RGBA8_data = image.getDataRgba8();
  return Status::OK;
}

Status Image::serializeToTga(core::memory::Blob &buffer) const {
  TGAImage image;
  image.getMutableDataRgba8(sizeX(), sizeY()) = m_RGBA8_data;
  return image.saveToMemory(buffer);
}

size_t Image::getSizeTga() const {
  return sizeof(TGAImage::Header) + 4 * sizeX() * sizeY();
}

} // namespace resources
} // namespace fsengine
