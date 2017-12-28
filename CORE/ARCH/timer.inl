#ifndef FISHY_TIMER_INL
#define FISHY_TIMER_INL

/**
 * Global high performance timer
 */
extern core::timer::PerformanceTimer g_timer;

/**
 *
 */
inline u64 core::timer::PerformanceTimer::getTotalTicks() {
  const u64 curTicks = timer::GetTicks();
  return curTicks - m_startTime;
}

/**
 *
 */
inline u64 core::timer::PerformanceTimer::getElapsedTicks() {
  const u64 curTicks = timer::GetTicks();
  const u64 elapsedTicks = curTicks - m_lastTime;
  m_lastTime = curTicks;
  return elapsedTicks;
}

/**
 *
 */
inline void core::timer::PerformanceTimer::reset() {
  m_lastTime = m_startTime = timer::GetTicks();
}

/**
 *
 */
inline u64 core::timer::GetResolutionTicks() {
  extern u64 g_timer_tick_per_sec;
  return g_timer_tick_per_sec;
}

/**
 *
 */
inline f64 core::timer::GetResolutionTime() {
  extern f64 g_timer_sec_per_tick;
  return g_timer_sec_per_tick;
}

/**
 *
 */
inline f64 core::timer::TicksToTime(u64 ticks) {
  return (f64) ticks * timer::GetResolutionTime();
}

/**
 *
 */
inline u64 core::timer::TimeToTicks(f64 time) {
  return (u64)(time / (f64) timer::GetResolutionTime());
}

#endif
