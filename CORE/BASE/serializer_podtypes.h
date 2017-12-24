/**
 * Serializers for basic types
 */
#ifndef FISHY_SERIALIZER_PODTYPES_H
#define FISHY_SERIALIZER_PODTYPES_H

#include <CORE/ARCH/endian.h>
#include <CORE/BASE/serializer.h>

OSERIALIZE(bool) {
  const u8 value = obj ? 1u : 0u;
  if (buff.write(core::memory::ConstBlob(&value, sizeof(u8))) != sizeof(u8)) {
    buff.set_fail();
  }
  return buff;
}
ISERIALIZE(bool) {
  u8 value = 0;
  core::memory::Blob tmp(&value, sizeof(u8));
  if (buff.read(tmp) != sizeof(u8)) {
    buff.set_fail();
  }
  obj = value != 0;
  return buff;
}

#define OSERIALIZE_POD(type)                                     \
  OSERIALIZE(type) {                                             \
    if (buff.write(core::memory::ConstBlob(                      \
            reinterpret_cast< const u8 * >(&obj), sizeof(type))) \
        != sizeof(type)) {                                       \
      buff.set_fail();                                           \
    }                                                            \
    return buff;                                                 \
  }

#define ISERIALIZE_POD(type)                                              \
  ISERIALIZE(type) {                                                      \
    core::memory::Blob tmp(reinterpret_cast< u8 * >(&obj), sizeof(type)); \
    if (buff.read(tmp) != sizeof(type)) {                                 \
      buff.set_fail();                                                    \
    }                                                                     \
    return buff;                                                          \
  }

OSERIALIZE_POD(u8);
ISERIALIZE_POD(u8);
OSERIALIZE_POD(s8);
ISERIALIZE_POD(s8);
#undef OSERIALIZE_POD
#undef ISERIALIZE_POD

#define OSERIALIZE_POD(type)                                     \
  OSERIALIZE(type) {                                             \
    const type tmp = core::endian::little(obj);                  \
    if (buff.write(core::memory::ConstBlob(                      \
            reinterpret_cast< const u8 * >(&tmp), sizeof(type))) \
        != sizeof(type)) {                                       \
      buff.set_fail();                                           \
    }                                                            \
    return buff;                                                 \
  }

#define ISERIALIZE_POD(type)                                              \
  ISERIALIZE(type) {                                                      \
    core::memory::Blob tmp(reinterpret_cast< u8 * >(&obj), sizeof(type)); \
    if (buff.read(tmp) != sizeof(type)) {                                 \
      buff.set_fail();                                                    \
    }                                                                     \
    obj = core::endian::little(obj);                                      \
    return buff;                                                          \
  }

OSERIALIZE_POD(u16);
ISERIALIZE_POD(u16);
OSERIALIZE_POD(s16);
ISERIALIZE_POD(s16);

OSERIALIZE_POD(u32);
ISERIALIZE_POD(u32);
OSERIALIZE_POD(s32);
ISERIALIZE_POD(s32);

OSERIALIZE_POD(u64);
ISERIALIZE_POD(u64);
OSERIALIZE_POD(s64);
ISERIALIZE_POD(s64);

OSERIALIZE_POD(f32);
ISERIALIZE_POD(f32);

OSERIALIZE_POD(f64);
ISERIALIZE_POD(f64);

#undef OSERIALIZE_POD
#undef ISERIALIZE_POD

OSERIALIZE(VarUInt) {
  u64 value = obj.get();
  u32 bit = 0;
  u8 bits[12];
  while (value >> 7) {
    bits[bit++] = (value & 0x7F) | 0x80;
    value = value >> 7;
  }
  bits[bit++] = value & 0x7F;
  if (buff.write(core::memory::ConstBlob(bits, bit)) != bit) {
    buff.set_fail();
  }
  return buff;
}

ISERIALIZE(VarUInt) {
  u64 nv = 0;
  u32 shift = 0;
  u8 fragment;
  do {
    buff >> fragment;
    const u64 fragV = (((u64) fragment) & 0x7F) << shift;
    nv = nv | fragV;
    shift += 7;
  } while (!buff.fail() && (fragment & 0x80));
  obj = VarUInt(nv);
  return buff;
}

OSERIALIZE(VarInt) {
  buff << VarUInt(static_cast< u64 >((obj.get() << 1) ^ (obj.get() >> 63)));
  return buff;
}

ISERIALIZE(VarInt) {
  VarUInt nv;
  buff >> nv;
  obj = VarInt(static_cast< s64 >((nv.get() >> 1) ^ (~(nv.get() & 1) + 1)));
  return buff;
}

#endif
