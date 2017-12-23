/**
 * Serializers for std::string
 */
#ifndef FISHY_SERIALIZER_STRINGS_H
#define FISHY_SERIALIZER_STRINGS_H

#include <CORE/BASE/serializer_podtypes.h>

#include <string>

OSERIALIZE(std::string) {
  const VarUInt sz(obj.size());
  buff << sz;
  if (buff.fail()) {
    return buff;
  }
  if (buff.write(core::memory::ConstBlob((const u8 *) obj.data(), obj.size()))
      != obj.size()) {
    buff.set_fail();
  }
  return buff;
}

ISERIALIZE(std::string) {
  VarUInt sz(0u);
  buff >> sz;
  if (buff.fail() || sz.get() >= std::numeric_limits< u32 >::max()) {
    return buff;
  }
  char *tmpStr = new char[(u32) sz.get()];
  core::memory::Blob tmp((u8 *) tmpStr, (u32) sz.get());
  if (buff.read(tmp) != sz.get()) {
    buff.set_fail();
  }
  obj = tmpStr;
  delete tmpStr;
  return buff;
}

#endif
