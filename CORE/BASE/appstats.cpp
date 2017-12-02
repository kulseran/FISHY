#include "appstats.h"

#include <CORE/ARCH/intrinsics.h>
#include <CORE/UTIL/stringutil.h>

#include <algorithm>
#include <mutex>

using core::util::IdentifierSafe;

namespace core {

static const u32 MAX_STATS = 1024;
static const u32 MAX_INFO = 128;

/**
 *
 */
static std::mutex &GetGlobalMutex() {
  static std::mutex s_mutex;
  return s_mutex;
}

/**
 *
 */
static tAppStatList &GetGlobalStats() {
  static tAppStatList s_list = tAppStatList(MAX_STATS);
  return s_list;
}

/**
 *
 */
static tAppInfoList &GetGlobalInfo() {
  static tAppInfoList s_list = tAppInfoList(MAX_INFO);
  return s_list;
}

/**
 * Helper lambda to match AppStat
 */
class StatFinder : public std::unary_function< const tAppStat &, bool > {
  public:
  StatFinder(const char *pName) : m_pName(pName) {}
  bool operator()(const core::tAppStat &stat) {
    if (stat.first == m_pName) {
      return true;
    }
    if (stat.first == nullptr) {
      return false;
    }
    ASSERT(m_pName);
    return strcmp(stat.first, m_pName) == 0;
  }

  private:
  const char *m_pName;
};

/**
 * Helper lambda to match AppInfo.
 */
class InfoFinder : public std::unary_function< const tAppInfo &, bool > {
  public:
  InfoFinder(const char *pName) : m_pName(pName) {}
  bool operator()(const core::tAppInfo &stat) {
    if (stat.first == m_pName) {
      return true;
    }
    if (stat.first == nullptr) {
      return false;
    }
    ASSERT(m_pName);
    return strcmp(stat.first, m_pName) == 0;
  }

  private:
  const char *m_pName;
};

/**
 *
 */
bool operator==(const core::tAppInfo &info, const char *pName) {
  return info.first == pName;
}

/**
 *
 */
AppStat::AppStat() : m_pCounter(nullptr) {
}

/**
 *
 */
AppStat::AppStat(const char *pName) {
  ASSERT(IdentifierSafe(pName) == pName);
  std::mutex &m = GetGlobalMutex();
  std::unique_lock< std::mutex > lock(m);

  tAppStatList &stats = GetGlobalStats();
  tAppStatList::iterator itr =
      std::find_if(stats.begin(), stats.end(), StatFinder(pName));
  if (itr == stats.end()) {
    itr = std::find_if(stats.begin(), stats.end(), StatFinder(nullptr));
    ASSERT(itr != stats.end());
    itr->first = pName;
  }
  m_pCounter = &itr->second;
}

/**
 *
 */
AppInfo::AppInfo() {
}

/**
 *
 */
AppInfo::AppInfo(const char *pName) {
  init(pName, nullptr);
}

/**
 *
 */
AppInfo::AppInfo(const char *pName, const char *pValue) {
  init(pName, pValue);
}

/**
 *
 */
void AppInfo::init(const char *pName, const char *pValue) {
  ASSERT(IdentifierSafe(pName) == pName);
  std::mutex &m = GetGlobalMutex();
  std::unique_lock< std::mutex > lock(m);

  tAppInfoList &info = GetGlobalInfo();
  tAppInfoList::iterator itr =
      std::find_if(info.begin(), info.end(), InfoFinder(pName));
  if (itr == info.end()) {
    itr = std::find_if(info.begin(), info.end(), InfoFinder(nullptr));
    ASSERT(itr != info.end());
    itr->first = pName;
  }
  m_pInfoStr = &itr->second;
  if (pValue != nullptr) {
    m_pInfoStr->assign(pValue);
  }
}

/**
 *
 */
void AppStat::increment() {
  ATOMIC_INCREMENT(m_pCounter);
}

/**
 *
 */
void AppStat::increment(const u32 amount) {
  u32 oldValue;
  u32 newValue;
  do {
    oldValue = *m_pCounter;
    newValue = oldValue + amount;
  } while (ATOMIC_COMPARE_EXCHANGE_32(m_pCounter, oldValue, newValue)
           != oldValue);
}

/**
 *
 */
void AppStat::reset() {
  u32 oldValue;
  do {
    oldValue = *m_pCounter;
  } while (ATOMIC_COMPARE_EXCHANGE_32(m_pCounter, oldValue, 0u) != oldValue);
}

/**
 *
 */
u32 AppStat::get() const {
  return *m_pCounter;
}

/**
 *
 */
void AppInfo::set(const std::string &value) {
  std::mutex &m = GetGlobalMutex();
  std::unique_lock< std::mutex > lock(m);
  *m_pInfoStr = value;
}

/**
 *
 */
void AppInfo::reset() {
  std::mutex &m = GetGlobalMutex();
  std::unique_lock< std::mutex > lock(m);
  m_pInfoStr->clear();
}

/**
 *
 */
std::string AppInfo::get() const {
  std::mutex &m = GetGlobalMutex();
  std::unique_lock< std::mutex > lock(m);
  return *m_pInfoStr;
}

/**
 *
 */
tAppStatList GetAllAppStats() {
  std::mutex &m = GetGlobalMutex();
  m.lock();
  tAppStatList copy = GetGlobalStats();
  m.unlock();
  return copy;
}

/**
 *
 */
tAppInfoList GetAllAppInfo() {
  std::mutex &m = GetGlobalMutex();
  m.lock();
  tAppInfoList copy = GetGlobalInfo();
  m.unlock();
  return copy;
}

} // namespace core
