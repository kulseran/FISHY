/**
 * High performance timers
 */
#ifndef FISHY_TIMER_H
#define FISHY_TIMER_H

#include <CORE/types.h>
#include <string>

namespace core {
namespace timer {

/**
 * Accessor to system's high performance timer
 */
class PerformanceTimer {
  public:
  PerformanceTimer();
  ~PerformanceTimer();

  inline u64 getTotalTicks();
  inline u64 getElapsedTicks();

  inline void reset();

  private:
  u64 m_startTime;
  u64 m_lastTime;
};

/**
 * Get the actual system ticks since application start.
 */
u64 GetTicks();

/**
 * Get ticks/sec resolution of the system timer.
 */
inline u64 GetResolutionTicks();

/**
 * Get how many seconds each tick represents
 */
inline f64 GetResolutionTime();

/**
 * Convert ticks to seconds.
 */
inline f64 TicksToTime(u64 ticks);

/**
 * Convert seconds to ticks.
 */
inline u64 TimeToTicks(f64 timeSeconds);

/**
 * Convert seconds to string
 */
std::string FormatTime(const f64 seconds);

/**
 * Get the system time as a string
 */
std::string GetSystemTime();

/**
 * Get the system date as a string
 */
std::string GetSystemDate();

/**
 * Microseconds since epoc
 */
u64 GetSystemTicks();

} // namespace timer

/**
 * Utility functions to convert between time units.
 */
namespace timeunit {
struct Unit {
  Unit(u32 micros, u32 seconds)
      : m_micros(micros),
        m_seconds(seconds),
        m_fraction((f64) seconds / (f64) micros) {}
  u64 m_micros;
  u64 m_seconds;
  f64 m_fraction;

  inline f32 toMicros(const f32 t) const { return (f32) toMicros((f64) t); }
  inline f64 toMicros(const f64 t) const {
    return t * (m_fraction * 1000000.0);
  }
  inline u32 toMicros(const u32 t) const { return (u32) toMicros((u64) t); }
  inline u64 toMicros(const u64 t) const {
    return (t * m_seconds * 1000000) / m_micros;
  }

  inline f32 toMillis(const f32 t) const { return (f32) toMillis((f64) t); }
  inline f64 toMillis(const f64 t) const { return t * (m_fraction * 1000.0); }
  inline u32 toMillis(const u32 t) const { return (u32) toMillis((u64) t); }
  inline u64 toMillis(const u64 t) const {
    return (t * m_seconds * 1000) / m_micros;
  }

  inline f32 toSeconds(const f32 t) const { return (f32) toSeconds((f64) t); }
  inline f64 toSeconds(const f64 t) const { return t * m_fraction; }
  inline u32 toSeconds(const u32 t) const { return (u32) toSeconds((u64) t); }
  inline u64 toSeconds(const u64 t) const { return (t * m_seconds) / m_micros; }

  inline f32 toMinutes(const f32 t) const { return (f32) toMinutes((f64) t); }
  inline f64 toMinutes(const f64 t) const { return t * (m_fraction / 60.0); }
  inline u32 toMinutes(const u32 t) const { return (u32) toMinutes((u64) t); }
  inline u64 toMinutes(const u64 t) const {
    return ((t * m_seconds) / 60) / m_micros;
  }

  inline f32 toHours(const f32 t) const { return (f32) toHours((f64) t); }
  inline f64 toHours(const f64 t) const { return t * (m_fraction / 3600.0); }
  inline u32 toHours(const u32 t) const { return (u32) toHours((u64) t); }
  inline u64 toHours(const u64 t) const {
    return ((t * m_seconds) / 3600) / m_micros;
  }

  inline f32 toDays(const f32 t) const { return (f32) toDays((f64) t); }
  inline f64 toDays(const f64 t) const {
    return t * (m_fraction / (24.0 * 3600.0));
  }
  inline u32 toDays(const u32 t) const { return (u32) toDays((u64) t); }
  inline u64 toDays(const u64 t) const {
    return ((t * m_seconds) / (24 * 3600)) / m_micros;
  }
};

static const Unit Micros(1000000, 1);
static const Unit Millis(1000, 1);
static const Unit Seconds(1, 1);
static const Unit Minute(1, 60);
static const Unit Hours(1, 3600);
static const Unit Days(1, 24 * 3600);
}; // namespace timeunit

} // namespace core

#  include "timer.inl"

#endif
