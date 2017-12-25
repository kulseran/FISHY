#include "vfs_util.h"

#include <CORE/UTIL/lexical_cast.h>

#include <atomic>

namespace vfs {
namespace util {

/**
 *
 */
Path GetTempDir() {
  return Path(TMP_PATH);
}

/**
 *
 */
Path GetTempFile() {
  static std::atomic_int32_t uniqueFileCounter = 0;
  std::string fileName;
  core::util::lexical_cast(uniqueFileCounter++, fileName).ignoreErrors();
  return Path("tmp/" + fileName);
}

} // namespace util
} // namespace vfs
