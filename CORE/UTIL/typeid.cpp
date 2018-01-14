#include "typeid.h"

#include <CORE/ARCH/intrinsics.h>

namespace core {
namespace util {

#if USE_BUILTIN_TYPEID
static Signature64 g_baseSignature = 0;
Signature64 getUniqueInstanceSignature() {
  ATOMIC_INCREMENT(&g_baseSignature);
  return g_baseSignature;
}
#endif

} // namespace util
} // namespace core
