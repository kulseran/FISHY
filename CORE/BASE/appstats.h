/**
 * Global stat tracking
 */
#ifndef FISHY_APPSTATS_H
#define FISHY_APPSTATS_H

#include <CORE/types.h>

#include <string>
#include <vector>

namespace core {

/**
 * AppStat represents a reference to a globally unique named counter.
 */
class AppStat {
  public:
  AppStat();

  /**
   * Create an AppStat referencing the given statistic name.
   *
   * @param name the name of the referenced statistic
   */
  AppStat(const char *name);

  /**
   * Atomically increment this statistic.
   */
  void increment();

  /**
   * Atomically increment this statistic.
   *
   * @param the amount to increment by.
   */
  void increment(const u32 amount);

  /**
   * Atomically reset this statistic to 0.
   */
  void reset();

  /**
   * Retrieve the current value.
   */
  u32 get() const;

  private:
  volatile u32 *m_pCounter;
};

/**
 * AppInfo represents a reference to a globally unique named string.
 */
class AppInfo {
  public:
  AppInfo();

  /**
   * Create an AppInfo referencing the given info name.
   *
   * @param name the name of the referenced info
   */
  AppInfo(const char *name);

  /**
   * Create an AppInfo referencing the given info name, and set the value to the
   * given string.
   *
   * @param name the name of the referenced info
   * @param value the constant string to set the info to
   */
  AppInfo(const char *name, const char *value);

  /**
   * Atomically the info string.
   */
  void set(const std::string &);

  /**
   * Atomically clear the info string.
   */
  void reset();

  /**
   * Retrieve the current value.
   */
  std::string get() const;

  private:
  void init(const char *name, const char *value);
  std::string *m_pInfoStr;
};

typedef std::pair< const char *, u32 > tAppStat;
typedef std::vector< tAppStat > tAppStatList;
typedef std::pair< const char *, std::string > tAppInfo;
typedef std::vector< tAppInfo > tAppInfoList;

/**
 * Get a copy of all registered {@link AppStat}
 */
tAppStatList GetAllAppStats();

/**
 * Get a copy of all registered {@link AppInfo}
 */
tAppInfoList GetAllAppInfo();

} // namespace core

#endif
