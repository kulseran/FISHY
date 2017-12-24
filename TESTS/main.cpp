/**
 * Unit test harness
 */
#include <iostream>

#include ".generated/version.h"

#include "testcase.h"

#include <CORE/BASE/asserts.h>
#include <CORE/BASE/logging.h>
#include <CORE/VFS/vfs.h>

int main(int argc, char **argv) {
  core::logging::RegisterSink(
      std::make_shared< core::logging::iLogSink >(~core::types::BitSet< LL >()))
      .ignoreErrors();

  vfs::Mount("./", "./");

  std::cout << "# Testing for build: " << BUILD_BRANCH_ID << "@"
            << BUILD_VERSION_HASH << " built on " << BUILD_TIMESTAMP
            << std::endl;
  std::cout << "# Running tests..." << std::endl;
  const int errorCount = testing::runRegisteredTests();
  std::cout << "# Done. With " << errorCount << " errors." << std::endl;

  vfs::UnmountAll();

  ASSERT(errorCount == 0);
  return ((errorCount == 0) ? 0 : 1);
}
