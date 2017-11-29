/**
 *
 */
#ifndef FISHY_CONCURRENT_QUEUE
#define FISHY_CONCURRENT_QUEUE

#include <CORE/BASE/status.h>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <vector>

namespace core {
namespace types {

template < typename tType >
class ConcurrentQueue {
  public:
  ConcurrentQueue(size_t maxSize = std::numeric_limits< size_t >::max());

  void push(const tType &);

  template < typename tIter >
  void pushN(const tIter &begin, const tIter &end);

  Status pop(tType &out);

  Status popN(const size_t count, std::vector< tType > &out);

  size_t size() const;

  void waitEmpty() const;

  void close();

  private:
  mutable std::mutex m_mutex;
  mutable std::condition_variable m_producerCV;
  mutable std::condition_variable m_consumerCV;
  std::atomic_bool m_open;

  std::deque< tType > m_queue;
  size_t m_maxSize;
};

template < typename tType >
ConcurrentQueue< tType >::ConcurrentQueue(size_t maxSize)
    : m_maxSize(maxSize), m_open(true) {
}

template < typename tType >
inline void ConcurrentQueue< tType >::push(const tType &val) {
  std::unique_lock< std::mutex > lock(m_mutex);
  while (m_open.load()) {
    if (m_queue.size() < m_maxSize) {
      m_queue.push_back(val);
      m_consumerCV.notify_one();
      break;
    } else {
      m_producerCV.wait(lock);
    }
  }
}

template < typename tType >
inline Status ConcurrentQueue< tType >::pop(tType &out) {
  std::unique_lock< std::mutex > lock(m_mutex);
  while (m_open.load() && m_queue.empty()) {
    m_consumerCV.wait(lock);
  }
  if (!m_open.load()) {
    m_producerCV.notify_all();
    return Status(Status::CANCELED);
  }
  out = m_queue.front();
  m_queue.pop_front();
  if (m_queue.empty()) {
    m_producerCV.notify_all();
  }
  return Status::ok();
}

template < typename tType >
template < typename tIter >
inline void
ConcurrentQueue< tType >::pushN(const tIter &begin, const tIter &end) {
  std::unique_lock< std::mutex > lock(m_mutex);
  for (typename tIter itr = begin; itr != end && m_open.load(); ++itr) {
    if (m_queue.size() < m_maxSize) {
      m_queue.push_back(val);
    } else {
      m_consumerCV.notify_all();
      m_producerCV.wait(lock);
    }
  }
  m_consumerCV.notify_all();
}

template < typename tType >
inline Status
ConcurrentQueue< tType >::popN(const size_t count, std::vector< tType > &out) {
  std::unique_lock< std::mutex > lock(m_mutex);
  while (m_open.load() && m_queue.empty()) {
    m_consumerCV.wait(lock);
  }
  if (!m_open.load()) {
    m_producerCV.notify_all();
    return Status(Status::CANCELED);
  }
  while (!m_queue.empty() && out.size() < count) {
    out.push_back(m_queue.front());
    m_queue.pop_front();
  }
  if (m_queue.empty()) {
    m_producerCV.notify_all();
  }
  return Status::ok();
}

template < typename tType >
inline size_t ConcurrentQueue< tType >::size() const {
  std::unique_lock< std::mutex > lock(m_mutex);
  return m_queue.size();
}

template < typename tType >
inline void ConcurrentQueue< tType >::waitEmpty() const {
  std::unique_lock< std::mutex > lock(m_mutex);
  while (!m_queue.empty() && m_open.load()) {
    m_producerCV.wait(lock);
  }
}

template < typename tType >
inline void ConcurrentQueue< tType >::close() {
  std::unique_lock< std::mutex > lock(m_mutex);
  ASSERT(m_queue.empty());
  m_open.store(false);
  m_consumerCV.notify_all();
}

} // namespace types
} // namespace core

#endif
