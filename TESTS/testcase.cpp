#include "testcase.h"
#include "test_assertions.h"

#include <CORE/UTIL/stringutil.h>

#include <cstring>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <vector>

namespace testing {

/**
 * Container for a test and info about which file it came from
 */
struct TestInfo {
  TestInfo(const char *file, const char *name, tTestFunc &func)
      : m_file(file), m_name(name), m_func(func) {}

  std::string m_file;
  std::string m_name;
  tTestFunc m_func;
};

/**
 * Global Test Registry
 */
static std::vector< TestInfo > &getTestRegistry() {
  static std::vector< TestInfo > registry;
  return registry;
}

/**
 * Register a function as a named testcase
 */
void registerTestCase(const char *file, const char *name, tTestFunc func) {
  getTestRegistry().push_back(TestInfo(file, name, func));
}

/**
 * Checks that the test wasn't registered twice
 */
void assertTestNotExist(
    const std::string &name, const std::set< std::string > &nameSet) {
  if (nameSet.find(name) == nameSet.end()) {
    return;
  }
  static char errMsg[2048] = {0};
  std::strncat(errMsg, "Test case registered twice: ", 2047);
  std::strncat(errMsg, name.c_str(), 2047 - std::strlen(errMsg));
  throw TestAssertionException(errMsg);
}

/**
 * Validate registered tests.
 */
void checkTests() {
  const std::vector< TestInfo > &registry = getTestRegistry();

  std::set< std::string > nameSet;
  for (std::vector< TestInfo >::const_iterator itr = registry.begin();
       itr != registry.end();
       ++itr) {
    TEST(assertTestNotExist(itr->m_file + itr->m_name, nameSet))
    nameSet.insert(itr->m_name);
  }
}

/**
 * Run all registered test cases.
 */
int runRegisteredTests() {
  registerTestCase(__FILE__, "check tests", checkTests);
  int errorCount = 0;
  const std::vector< TestInfo > &registry = getTestRegistry();
  const size_t sz = registry.size();
  std::cout << "1.." << sz << std::endl;
  for (std::vector< TestInfo >::const_iterator itr = registry.begin();
       itr != registry.end();
       ++itr) {
    const size_t currentId = std::distance(registry.begin(), itr) + 1;
    const float percent = 100.0f * ((float) currentId / (float) sz);
    try {
      itr->m_func();
      std::cout << "ok " << currentId << " (" << percent << "% done)"
                << std::endl;
    } catch (testing::TestAssertionException e) {
      std::cout << "not ok " << currentId << " Test failure at [" << e.file()
                << ":" << e.line() << " @ " << e.func() << "] > "
                << core::util::Escape(e.what()) << std::endl;
      errorCount++;
    }
  }
  std::cout << "# Done testing." << std::endl;
  return errorCount;
}

} // namespace testing
