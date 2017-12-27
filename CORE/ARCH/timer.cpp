#include "timer.h"

#if defined(PLAT_WIN32)
#  include <windows.h>
#  pragma comment(lib, "Winmm.lib")
#elif defined(PLAT_LINUX)
#  include <sys/time.h>
#endif

#include <ctime>
#include <sstream>

#include <chrono>

namespace core {
namespace timer {

bool g_timer_PerformanceTimerEnabled = false;
f64 g_timer_sec_per_tick = 1.0f / f64(CLOCKS_PER_SEC);
u64 g_timer_tick_per_sec = CLOCKS_PER_SEC;

class SetupTimeFloat {
  public:
  SetupTimeFloat() {
#ifdef PLAT_WIN32
    if (QueryPerformanceFrequency((LARGE_INTEGER *) &g_timer_tick_per_sec)) {
      g_timer_sec_per_tick = ((f64) 1.0) / ((f64) g_timer_tick_per_sec);
      g_timer_PerformanceTimerEnabled = true;
    }
#endif
#ifdef linux
    timespec tp;
    if (clock_getres(CLOCK_MONOTONIC, &tp) == 0) {
      g_timer_sec_per_tick = (f64) tp.tv_nsec / 1000000000.0f;
      g_timer_tick_per_sec = (u64)(1.0f / g_timer_sec_per_tick);
      g_timer_PerformanceTimerEnabled = true;
    }
#endif
    m_ranSetup = true;
  }

  operator bool() const { return m_ranSetup; }

  private:
  bool m_ranSetup;
};
static SetupTimeFloat g_timerSetup;

/**
 *
 */
u64 GetTicks(void) {
  u64 curTicks;
  if (g_timer_PerformanceTimerEnabled) {
#ifdef WIN32
    QueryPerformanceCounter((LARGE_INTEGER *) &curTicks);
#endif
#ifdef linux
    timespec tp;
    int rVal = clock_gettime(CLOCK_MONOTONIC, &tp);
    ASSERT(rVal == 0);
    curTicks = ((u64) tp.tv_nsec) + (1000000000 * ((u64) tp.tv_sec));
#endif
  } else {
    curTicks = (u64) clock();
  }
  return curTicks;
}

//__________
PerformanceTimer::PerformanceTimer() {
  ASSERT(g_timerSetup);
  reset();
}

/**
 *
 */
PerformanceTimer::~PerformanceTimer() {
}

/**
 *
 */
std::string GetSystemTime() {
  char cptime[50];
  time_t now = time(NULL);
  strftime(cptime, 50, "%H:%M:%S", localtime(&now));
  return cptime;
}

/**
 *
 */
std::string GetSystemDate() {
  char cptime[50]; // char ptr
  time_t now = time(NULL);
  strftime(cptime, 50, "%x", localtime(&now)); // uses short month name
  return cptime;
}

#if defined(PLAT_WIN32)
struct timeval {
  u32 tv_sec;
  u32 tv_usec;
};

/* FILETIME of Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = 116444736000000000ULL;
/**
 * Helper wrapper since gettimeofday doesn't exist on win32
 */
static int gettimeofday(struct timeval *tp, struct timezone *tzp) {
  FILETIME file_time;
  SYSTEMTIME system_time;
  ULARGE_INTEGER ularge;
  GetSystemTime(&system_time);
  SystemTimeToFileTime(&system_time, &file_time);
  ularge.LowPart = file_time.dwLowDateTime;
  ularge.HighPart = file_time.dwHighDateTime;

  tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
  tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
  return 0;
}
#endif

/**
 *
 */
u64 GetSystemTicks() {
  timeval tv;
  gettimeofday(&tv, NULL);

  u64 now = ((u64) tv.tv_sec) * 1000000L;
  now += tv.tv_usec;
  return now;
}

/**
 *
 */
std::string FormatTime(f64 seconds) {
  static const f64 SECS_PER_MIN = 60;
  static const f64 SECS_PER_HOUR = 60 * SECS_PER_MIN;
  static const f64 SECS_PER_DAY = 24 * SECS_PER_HOUR;

  std::stringstream rVal;

  if (seconds > SECS_PER_DAY) {
    u64 days = (u64)(seconds / SECS_PER_DAY);
    seconds -= SECS_PER_DAY * days;
    rVal << days << "d ";
  }
  if (seconds > SECS_PER_HOUR) {
    u64 days = (u64)(seconds / SECS_PER_HOUR);
    seconds -= SECS_PER_HOUR * days;
    rVal << days << "h ";
  }
  if (seconds > SECS_PER_MIN) {
    u64 days = (u64)(seconds / SECS_PER_MIN);
    seconds -= SECS_PER_MIN * days;
    rVal << days << "m ";
  }

  rVal << seconds << "s";

  return rVal.str();
}

} // namespace timer
} // namespace core

/**
 * Global timer
 */
core::timer::PerformanceTimer g_timer;
