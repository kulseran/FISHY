#ifndef FISHY_CONFIG_INL
#define FISHY_CONFIG_INL

#include <CORE/UTIL/lexical_cast.h>

namespace core {
namespace config {

/**
 *
 */
template < typename tType >
Flag< tType >::Flag(const char *name, const tType &defaultValue)
    : iFlagBase(name, ""), m_value(defaultValue) {
}

/**
 *
 */
template < typename tType >
Flag< tType >::Flag(
    const char *name, const char *desc, const tType &defaultValue)
    : iFlagBase(name, desc), m_value(defaultValue) {
}

/**
 *
 */
template < typename tType >
std::string Flag< tType >::toString() const {
  std::string rVal("<unknown>");
  util::lexical_cast(m_value, rVal);
  return rVal;
}

/**
 *
 */
template < typename tType >
bool Flag< tType >::fromString(const std::string &value) {
  if (util::lexical_cast(value, m_value)) {
    m_set = true;
    return true;
  }
  return false;
}

} // namespace config
} // namespace core

/**
 *
 */
template < typename tEnum >
inline std::stringstream &operator<<(
    std::stringstream &stream, const core::config::FlagEnum< tEnum > &flag) {
  stream << tEnum::enumNames[flag.m_value];
  return stream;
}

/**
 *
 */
template < typename tEnum >
inline std::stringstream &
operator>>(std::stringstream &stream, core::config::FlagEnum< tEnum > &flag) {
  std::string name;
  stream >> name;
  for (int i = 0; i < tEnum::COUNT; ++i) {
    if (name == tEnum::enumNames[i]) {
      flag.m_value = (typename tEnum::type) i;
      break;
    }
  }
  return stream;
}

#endif
