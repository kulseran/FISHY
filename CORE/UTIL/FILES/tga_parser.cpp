#include "tga_parser.h"

#include <CORE/ARCH/endian.h>
#include <CORE/BASE/checks.h>
#include <CORE/BASE/logging.h>

namespace core {
namespace util {
namespace files {

static const TGAImage::Header g_defaultHeader = {
    0, 0, TGAImage::RGB, 0, 0, 0, 0, 0, 0, 0, 32, 0};

TGAImage::TGAImage() : m_header(g_defaultHeader) {
}

/**
 *
 */
Status TGAImage::loadFromMemory(const core::memory::ConstBlob &buffer) {
  Trace();
  const u8 *dataPtr = buffer.data();
  const size_t sz = buffer.size();

  // The buffer needs enough data for this to be a tga
  if (sz < sizeof(Header)) {
    return Status::BAD_INPUT;
  }
  const Header *pHeader = reinterpret_cast< const Header * >(dataPtr);
  Header header = *pHeader;

  // Swap header bytes if needed
  header.m_paletteOrigin = core::endian::little(header.m_paletteOrigin);
  header.m_paletteEntries = core::endian::little(header.m_paletteEntries);
  header.m_xPos = core::endian::little(header.m_xPos);
  header.m_yPos = core::endian::little(header.m_yPos);
  header.m_width = core::endian::little(header.m_width);
  header.m_height = core::endian::little(header.m_height);

  // Good dimentions?
  RET_SM(
      (header.m_width > 0 && header.m_height > 0),
      Status::BAD_INPUT,
      "Image has bad size.");

  // Only support 32bit images
  RET_SM(
      header.m_bitsPerPixel == 32,
      Status::UNSUPPORTED,
      "Image has bad bitdepth.");

  // Only support full-color uncompressed images
  RET_SM(
      header.m_imageType == RGB,
      Status::UNSUPPORTED,
      "Image has bad format type.");

  // The buffer needs to have enough data in it for this to be a real image
  const size_t imageDataSz = 4 * header.m_width * header.m_height;
  RET_SM(
      (sz - sizeof(Header)) >= imageDataSz,
      Status::BAD_INPUT,
      "Image data size doesn't match dimensions.");

  // Copy in the image data
  const u8 *imageDataStart = dataPtr + sizeof(Header);
  const u8 *imageDataEnd = imageDataStart + imageDataSz;
  ASSERT(imageDataSz);
  m_RGBA8_data.resize(imageDataSz);

  // Flip the image depending on the "top corner" flag of the tga format.
  // If bit 5 of the descriptor is:
  // 0 - Origin in the lower left (matches opengl)
  // 1 - Origin in the upper left (needs flip)
  if ((header.m_descriptor & (1 << 5))) {
    u8 *dataBegin = &m_RGBA8_data[0];
    const size_t lineStride = header.m_width * 4;
    for (u32 i = 0; i < header.m_height; ++i) {
      memcpy(
          dataBegin + lineStride * i,
          imageDataStart + lineStride * (header.m_height - 1 - i),
          lineStride);
    }
  } else {
    memcpy(&m_RGBA8_data[0], imageDataStart, imageDataSz);
  }
  m_header = header;

  return Status::OK;
}

Status TGAImage::saveToMemory(core::memory::Blob &buffer) const {
  Trace();
  u8 *dataPtr = buffer.data();
  const size_t sz = buffer.size();

  RET_SM(
      m_header.m_width != 0 && m_header.m_height != 0,
      Status::BAD_INPUT,
      "TGA Image was empty");

  // The buffer needs enough data for this to be a tga
  RET_SM(
      sz >= getSaveSize(), Status::BAD_ARGUMENT, "TGA output buffer too small");

  const size_t imageDataSz = 4 * m_header.m_width * m_header.m_height;
  RET_SM(
      m_RGBA8_data.size() == imageDataSz,
      Status::BAD_STATE,
      "TGA Internal buffer incorrectly sized. Expected: "
          << imageDataSz << " Was: " << m_RGBA8_data.size());

  Header *pHeader = reinterpret_cast< Header * >(dataPtr);
  *pHeader = m_header;
  // Swap header bytes if needed
  pHeader->m_paletteOrigin = core::endian::little(pHeader->m_paletteOrigin);
  pHeader->m_paletteEntries = core::endian::little(pHeader->m_paletteEntries);
  pHeader->m_xPos = core::endian::little(pHeader->m_xPos);
  pHeader->m_yPos = core::endian::little(pHeader->m_yPos);
  pHeader->m_width = core::endian::little(pHeader->m_width);
  pHeader->m_height = core::endian::little(pHeader->m_height);

  // Copy in the image data
  u8 *imageDataStart = dataPtr + sizeof(Header);
  memcpy(imageDataStart, &m_RGBA8_data[0], imageDataSz);
  return Status::OK;
}

size_t TGAImage::getSaveSize() const {
  return sizeof(Header) + 4 * m_header.m_height * m_header.m_width;
}

std::vector< u8 > &
TGAImage::getMutableDataRgba8(const size_t sx, const size_t sy) {
  CHECK(sx < std::numeric_limits< u16 >::max());
  CHECK(sy < std::numeric_limits< u16 >::max());
  m_header.m_width = std::max(static_cast< u16 >(sx), m_header.m_width);
  m_header.m_height = std::max(static_cast< u16 >(sy), m_header.m_height);
  m_RGBA8_data.resize(4 * m_header.m_width * m_header.m_height);
  return m_RGBA8_data;
}

} // namespace files
} // namespace util
} // namespace core
