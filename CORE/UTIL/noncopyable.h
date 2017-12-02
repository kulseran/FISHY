/**
 * Prevents object from implementing copy operations.
 *   class foo : public noncopyable {
 *   };
 *
 */
#ifndef FISHY_NON_COPYABLE_H
#define FISHY_NON_COPYABLE_H

namespace core {
namespace util {

/**
 * Classes inheriting from this will result in compile errors
 * if they are attempted to be copied.
 */
class noncopyable {
  public:
  noncopyable() {}

  private:
  noncopyable(const noncopyable &) {}
  noncopyable &operator=(const noncopyable &) { return *this; }
};

} // namespace util
} // namespace core

#endif
