/**
 * Utility for converting a binary on/off image to
 * a signed distance field image.
 */
#ifndef FISHY_DISTANCE_FIELD_H
#define FISHY_DISTANCE_FIELD_H

#include <CORE/types.h>

#include <vector>

/**
 * Conversion utility for computing the signed distance field
 * representation of an input black and white image.
 */
class DistanceGrid {
  public:
  /**
   * Initialize a grid converter for a fixed image size and
   * distance scaling factor.
   *
   * @param sx horizontal width
   * @param sy vertical height
   * @param scale distance scaling factor
   */
  DistanceGrid(const u32 sx, const u32 sy, const f32 scale);

  /**
   * Perform a conversion
   *
   * @param pImageDataIn the input black-and-white image
   * @param pImageDataOut teh output greyscale image with 128 as the midpoint
   * color.
   */
  void convertToSdf(const u8 *pImageDataIn, u8 *pImageDataOut);

  private:
  struct DistancePoint {
    f32 m_dx;
    f32 m_dy;
    DistancePoint() : m_dx(0.0f), m_dy(0.0f) {}
    DistancePoint(f32 dx, f32 dy) : m_dx(dx), m_dy(dy) {}
    f32 dist() const { return m_dx * m_dx + m_dy * m_dy; }
  };

  s32 m_szx;
  s32 m_szy;
  f32 m_scale;
  std::vector< DistancePoint > m_gridInside;
  std::vector< DistancePoint > m_gridOutside;

  void fromRgba(const u8 *pImageData);
  void toRgba(u8 *pImageData);
  void computeSdf(std::vector< DistancePoint > &);

  int index(int x, int y) const;
  DistancePoint compare(
      const std::vector< DistancePoint > &,
      const DistancePoint &p,
      int x,
      int y) const;
};

#endif
