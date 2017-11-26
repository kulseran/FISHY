/**
 * Type used for variable length integer encodings.
 */
#ifndef FISHY_VARINT_TYPE_H
#define FISHY_VARINT_TYPE_H

/**
 * Type for naming a {@link u64} as a variable length unsigned integer for
 * serialization purposes.
 */
class VarUInt {
  public:
  VarUInt() : m_value(0) {}
  explicit VarUInt(const u8 value) : m_value(value) {}
  explicit VarUInt(const u16 value) : m_value(value) {}
  explicit VarUInt(const u32 value) : m_value(value) {}
  explicit VarUInt(const u64 value) : m_value(value) {}

  u64 get() const { return m_value; }

  bool operator==(const VarUInt &other) const {
    return m_value == other.m_value;
  }

  private:
  u64 m_value;
};

/**
 * Type for naming a {@link s64} as a variable length signed integer for
 * serialization purposes.
 */
class VarInt {
  public:
  VarInt() : m_value(0) {}
  explicit VarInt(const s8 value) : m_value(value) {}
  explicit VarInt(const s16 value) : m_value(value) {}
  explicit VarInt(const s32 value) : m_value(value) {}
  explicit VarInt(const s64 value) : m_value(value) {}

  s64 get() const { return m_value; }

  bool operator==(const VarInt &other) const {
    return m_value == other.m_value;
  }

  private:
  s64 m_value;
};

inline u64 encode_zigzag(s64 u) {
  return static_cast< u64 >((u << 1) ^ (u >> 63));
}

inline s64 decode_zigzag(u64 u) {
  return static_cast< s64 >((u >> 1) ^ (~(u & 1) + 1));
}

#endif
