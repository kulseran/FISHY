/**
 * Image Resource
 */
#ifndef FISHY_IMAGE_H
#define FISHY_IMAGE_H

#include <CORE/BASE/status.h>
#include <CORE/MEMORY/blob.h>
#include <CORE/types.h>

#include <algorithm>
#include <vector>

namespace fsengine {
namespace resources {

/**
 * Image holder class
 */
class Image {
  public:
  /**
   * Input file format type
   */
  enum FmtType { FORMAT_TGA, FORMAT_COUNT };

  /**
   * Load from a file buffer in memory. Makes a copy of the image data.
   */
  bool loadFromMemory(const FmtType fmt, const core::memory::ConstBlob &data);

  /**
   * Save to a buffer in memory that can be written as a correctly formatted
   * file.
   */
  bool saveToMemory(const FmtType fmt, core::memory::Blob &data) const;

  /**
   * @return the max size of the save buffer needed for {@link #saveToMemory}
   */
  size_t getSaveSize(const FmtType fmt) const;

  /**
   * @return the X dimention of the image
   */
  size_t sizeX() const { return m_szx; }

  /**
   * @return the Y dimention of the image
   */
  size_t sizeY() const { return m_szy; }

  /**
   * @return pointer to the 32bit image data in RGBA format
   */
  std::vector< u8 > &getMutableDataRgba8(size_t sx, size_t sy) {
    m_szx = std::max(sx, m_szx);
    m_szy = std::max(sy, m_szy);
    m_RGBA8_data.resize(4 * m_szx * m_szy);
    return m_RGBA8_data;
  }

  /**
   * @return pointer to the 32bit image data in RGBA format
   */
  const std::vector< u8 > &getDataRgba8() const { return m_RGBA8_data; }

  /**
   * @return true if the image contains data
   */
  bool isValid() const { return (m_RGBA8_data.size() != 0); }

  private:
  Status serializeFromTga(const core::memory::ConstBlob &);
  Status serializeToTga(core::memory::Blob &) const;
  size_t getSizeTga() const;

  std::vector< u8 > m_RGBA8_data;
  size_t m_szx, m_szy;
};

} // namespace resources
} // namespace fsengine
#endif
