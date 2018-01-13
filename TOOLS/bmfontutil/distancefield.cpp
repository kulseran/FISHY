#include "distancefield.h"

#include <CORE/MATH/math_limit.h>

/**
 *
 */
DistanceGrid::DistanceGrid(const u32 sx, const u32 sy, const f32 scale)
    : m_szx(sx), m_szy(sy), m_scale(scale) {
  m_gridInside.resize(sx * sy);
  m_gridOutside.resize(sx * sy);
}

/**
 *
 */
void DistanceGrid::fromRgba(const u8 *pImageData) {
  for (s32 i = 0; i < m_szx * m_szy; ++i) {
    const u8 r = *pImageData++;
    const u8 g = *pImageData++;
    const u8 b = *pImageData++;
    const u8 a = *pImageData++;

    if (a < 128) {
      m_gridInside[i] = DistancePoint(0.0f, 0.0f);
      m_gridOutside[i] = DistancePoint(1.0f, 1.0f);
    } else {
      m_gridInside[i] = DistancePoint(1.0f, 1.0f);
      m_gridOutside[i] = DistancePoint(0.0f, 0.0f);
    }
  }
}

/**
 *
 */
int DistanceGrid::index(int x, int y) const {
  x = core::math::clamp(x, 0, (m_szx - 1));
  y = core::math::clamp(y, 0, (m_szy - 1));
  return y * m_szx + x;
}

/**
 *
 */
DistanceGrid::DistancePoint DistanceGrid::compare(
    const std::vector< DistancePoint > &grid,
    const DistancePoint &p,
    int x,
    int y) const {
  const f32 stepX = 1.0f / m_szx;
  const f32 stepY = 1.0f / m_szy;

  DistancePoint other = grid[index(x, y)];
  other.m_dx += stepX;
  other.m_dy += stepY;

  if (other.dist() < p.dist()) {
    return other;
  }

  return p;
}

/**
 *
 */
void DistanceGrid::computeSdf(std::vector< DistancePoint > &grid) {
  for (int y = 0; y < m_szy; ++y) {
    for (int x = 0; x < m_szx; ++x) {
      DistancePoint p = grid[index(x, y)];
      p = compare(grid, p, x - 1, y);
      p = compare(grid, p, x, y - 1);
      p = compare(grid, p, x - 1, y - 1);
      p = compare(grid, p, x + 1, y - 1);
      grid[index(x, y)] = p;
    }
    for (int x = m_szx - 1; x >= 0; --x) {
      DistancePoint p = grid[index(x, y)];
      p = compare(grid, p, x + 1, y);
      grid[index(x, y)] = p;
    }
  }
  for (int y = m_szy - 1; y >= 0; --y) {
    for (int x = m_szx - 1; x >= 0; --x) {
      DistancePoint p = grid[index(x, y)];
      p = compare(grid, p, x + 1, y);
      p = compare(grid, p, x, y + 1);
      p = compare(grid, p, x - 1, y + 1);
      p = compare(grid, p, x + 1, y + 1);
      grid[index(x, y)] = p;
    }
    for (int x = 0; x < m_szx; ++x) {
      DistancePoint p = grid[index(x, y)];
      p = compare(grid, p, x - 1, y);
      grid[index(x, y)] = p;
    }
  }
}

/**
 *
 */
void DistanceGrid::convertToSdf(const u8 *pImageDataIn, u8 *pImageDataOut) {
  fromRgba(pImageDataIn);
  computeSdf(m_gridInside);
  computeSdf(m_gridOutside);
  toRgba(pImageDataOut);
}

/**
 *
 */
void DistanceGrid::toRgba(u8 *pImageData) {
  for (s32 i = 0; i < m_szx * m_szy; ++i) {
    const f32 distfloat =
        sqrt(m_gridInside[i].dist()) - sqrt(m_gridOutside[i].dist());
    const f32 distscaled = (distfloat / m_scale) + 0.5f;
    u8 dist = (u8) core::math::clamp(255.0f * distscaled, 0.0f, 255.0f);
    *pImageData++ = 255;  // r
    *pImageData++ = 255;  // g
    *pImageData++ = 255;  // b
    *pImageData++ = dist; // a
  }
}
