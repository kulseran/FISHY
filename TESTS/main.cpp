/**
 * Unit test harness
 */
#include <iostream>

#include ".generated/version.h"

#include "testcase.h"

#include <CORE/BASE/asserts.h>

int main(int argc, char **argv) {
  std::cout << "# Testing for build: " << BUILD_BRANCH_ID << "@"
            << BUILD_VERSION_HASH << " built on " << BUILD_TIMESTAMP
            << std::endl;
  std::cout << "# Running tests..." << std::endl;
  const int errorCount = testing::runRegisteredTests();
  std::cout << "# Done. With " << errorCount << " errors." << std::endl;

  ASSERT(errorCount == 0);
  return ((errorCount == 0) ? 0 : 1);
}
