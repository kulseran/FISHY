/**
 * Thread-safe Producer/Consumer queue with fixed size.
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
  /**
   * @param maxSize the max size of the queue before push operations block
   */
  ConcurrentQueue(size_t maxSize = std::numeric_limits< size_t >::max());

  /**
   * Push a new item into the queue.
   * This function blocks if the queue is full.
   */
  void push(const tType &);

  /**
   * Push many items into the queue using a minimal number of locking steps.
   * This function blocks if the queue is full.
   */
  template < typename tIter >
  void pushN(const tIter &begin, const tIter &end);

  /**
   * Pop an item from the queue.
   * This function blocks if the queue is empty.
   *
   * @return Status ok if an item was popped, or CANCELED if the queue was
   *     closed.
   */
  Status pop(tType &out);

  /**
   * Pop up to N items from the queue.
   * This function does not block if the queue is empty.
   *
   * @return Status ok if items were popped, or CANCELED if the queue was
   *     closed.
   */
  Status popN(const size_t count, std::vector< tType > &out);

  /**
   * @return the current size of the queue
   */
  size_t size() const;

  /**
   * @return if the queue is likely empty.
   */
  bool empty() const { return size() == 0; }

  /**
   * @return waits until all the items have been removed from the queue.
   */
  void waitEmpty() const;

  /**
   * Closes the queue. All blocking operations are released, and no more items
   * may be put into the queue.
   */
  void close();

  /**
   * Check if the queue is still open.
   */
  bool isOpen() const;

  private:
  mutable std::mutex m_mutex;
  mutable std::condition_variable m_producerCV;
  mutable std::condition_variable m_consumerCV;
  std::atomic_bool m_open;

  std::deque< tType > m_queue;
  size_t m_maxSize;
};

} // namespace types
} // namespace core

#  include "concurrent_queue.inl"

#endif
