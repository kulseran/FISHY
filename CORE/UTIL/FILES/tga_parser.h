/**
 * TGA Image Resource
 */
#ifndef FISHY_TGA_PARSER_H
#define FISHY_TGA_PARSER_H

#include <CORE/BASE/status.h>
#include <CORE/MEMORY/blob.h>
#include <CORE/types.h>

#include <algorithm>
#include <vector>

namespace core {
namespace util {
namespace files {

/**
 * Container for a TGA Image
 */
class TGAImage {
  public:
  /**
   * Possible storage formats for a Targa image
   */
  enum Formats {
    NO_TGA_DATA = 0,
    COLOR_MAPPED = 1,
    RGB = 2,
    BLACK_WHITE = 3,
    RLE_COLOR_MAPPED = 9,
    RLE_RGB = 10,
    COMP_BLACK_WHITE = 11,
    COMP_COLOR_MAPPED_HUFFMAN_DELTA_RLE = 32,
    COMP_COLOR_MAPPED_HUFFMAN_DELTA_RLE_4_PASS_QUADTREE = 33,
  };

  /**
   * Targa Image Header
   */
#pragma pack(push, 1)
  struct Header {
    u8 m_numIdChar;
    u8 m_paletteType;
    u8 m_imageType;
    u16 m_paletteOrigin;
    u16 m_paletteEntries;
    u8 m_paletteBitsPerEntry;
    u16 m_xPos;
    u16 m_yPos;
    u16 m_width;
    u16 m_height;
    u8 m_bitsPerPixel;
    u8 m_descriptor;
  };
#pragma pack(pop)

  TGAImage();

  /**
   * Load from a file buffer in memory. Makes a copy of the image data.
   */
  Status loadFromMemory(const ::core::memory::ConstBlob &data);

  /**
   * Save to a buffer in memory that can be written directly to disk as a
   * correctly formatted file.
   */
  Status saveToMemory(::core::memory::Blob &data) const;

  /**
   * @return the max size of the save buffer needed for {@link #saveToMemory}
   */
  size_t getSaveSize() const;

  /**
   * @return the Y dimention of the image
   */
  Header getHeader() const { return m_header; }

  /**
   * @return pointer to the 32bit image data in RGBA format
   */
  std::vector< u8 > &getMutableDataRgba8(const size_t sx, const size_t sy);

  /**
   * @return pointer to the 32bit image data in RGBA format
   */
  const std::vector< u8 > &getDataRgba8() const { return m_RGBA8_data; }

  /**
   * @return true if the image contains data
   */
  bool isValid() const { return (m_RGBA8_data.size() != 0); }

  private:
  std::vector< u8 > m_RGBA8_data;
  Header m_header;
};

} // namespace files
} // namespace util
} // namespace core
#endif
