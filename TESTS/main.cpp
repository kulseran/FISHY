/**
 * Unit test harness
 */
#include <iostream>

#include ".generated/version.h"

#include "testcase.h"

int main(int argc, char **argv) {
  std::clog << "# Testing for build: " << BUILD_BRANCH_ID << "@"
            << BUILD_VERSION_HASH << " built on " << BUILD_TIMESTAMP
            << std::endl;
  std::clog << "# Running tests..." << std::endl;
  const int errorCount = testing::runRegisteredTests();
  std::clog << "# Done. With " << errorCount << " errors." << std::endl;

  return ((errorCount == 0) ? 0 : 1);
}
