/**
 * Configuration parsing from commandline flags and config files.
 * Registers a single default flag --config_file which may be used
 * set flag values from a file with lines of the form "flag=value"
 *
 * Usage:
 * @see core::config::Flag
 *
 *   #include "config.h"
 *   core::config::Flag< type >  g_flagName("name", "desc", default);
 *
 *   int main(const int argc, const char **argv) {
 *     CHECK(core::config::ParseFlags(argc, argv));
 *     type something = g_flagName.get();
 *   }
 */
#ifndef FISHY_CONFIG_H
#define FISHY_CONFIG_H

#include <CORE/BASE/status.h>

#include <string>

namespace core {
namespace config {

/**
 * Parses commandline flags to complete any registered config variables. If the
 * commandline flag --config_file is set, this also parses the config file.
 */
Status ParseFlags(const int argc, const char **argv);

/**
 * Prints out all available flags to the command line, along with their current
 * value.
 */
void PrintFlags();

/**
 * Flag interface, all flag types must inherit from this
 */
class iFlagBase {
  public:
  iFlagBase(const char *name, const char *desc);
  virtual ~iFlagBase() {}

  virtual bool fromString(const std::string &value) = 0;
  virtual std::string toString() const = 0;

  const char *getName() const { return m_name; }
  const char *getDesc() const { return m_desc; }
  bool wasSet() const { return m_set; }
  void checkSet() const;

  private:
  const char *m_name;
  const char *m_desc;

  protected:
  bool m_set;
};

/**
 * Flag value that can be set from a config, or commandline
 */
template < typename tType >
class Flag : public iFlagBase {
  public:
  Flag(const char *name, const tType &defaultValue);
  Flag(const char *name, const char *desc, const tType &defaultValue);

  /**
   * Getter, returns default value or set value.
   * @see iFlagBase::wasSet()
   * @see iFlagBase::checkSet()
   */
  inline const tType &get() const { return m_value; }

  virtual bool fromString(const std::string &value);
  virtual std::string toString() const;

  private:
  tType m_value;
};

/**
 * Container for a enum, which allows it to be set by name from a flag. To
 * support this operation, the contained enum must be structured like:
 *
 *     struct eMyEnum {
 *       enum type {
 *         UNKNOWN,
 *         ...
 *         COUNT
 *       }
 *       static const char *enumNames[COUNT];
 *     };
 *     ...
 *     const char *eMyEnum::enumNames[eMyEnum::COUNT] = {
 *       "UNKNOWN",
 *       ...
 *     };
 */
template < typename tEnum >
class FlagEnum {
  public:
  FlagEnum() : m_value(tEnum::UNKNOWN) {}
  FlagEnum(const typename tEnum::type value) : m_value(value) {}

  typename tEnum::type m_value;
};

} // namespace config
} // namespace core

#  include "config.inl"

#endif
