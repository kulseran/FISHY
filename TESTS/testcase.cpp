/**
 * testcase.cpp
 *
 */
#include "testcase.h"
#include "test_assertions.h"

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
    : m_file(file),
      m_name(name),
      m_func(func) {
  }

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
void assertTestNotExist(const std::string &name, const std::set<std::string> &nameSet) {
  if (nameSet.find(name) == nameSet.end()) {
    return;
  }
  static char errMsg[2048];
  errMsg[0] = 0;
  strcat_s(errMsg, "Test case registered twice: ");
  strcat_s(errMsg, name.c_str());
  throw TestAssertionException(errMsg);
}

/**
 * Validate registered tests.
 */
void checkTests() {
  const std::vector< TestInfo > &registry = getTestRegistry();

  std::set<std::string> nameSet;
  for (std::vector< TestInfo >::const_iterator itr = registry.begin(); itr != registry.end(); ++itr) {
    TEST(assertTestNotExist(itr->m_name, nameSet))
    nameSet.insert(itr->m_name);
  }
}

/**
 * Escape text.
 */
std::string escape(const std::string &str) {
  std::string rVal;
  rVal.reserve(str.size());
  std::string::const_iterator itr = str.begin();
  while (itr != str.end()) {
    switch (*itr) {
      case '\n':
        rVal.push_back('\\');
        rVal.push_back('n');
        break;
      case '\r':
        rVal.push_back('\\');
        rVal.push_back('r');
        break;
      case '\t':
        rVal.push_back('\\');
        rVal.push_back('t');
        break;
      case '\"':
        rVal.push_back('\\');
        rVal.push_back('\"');
        break;
      case '\\':
        rVal.push_back('\\');
        rVal.push_back('\\');
        break;
      default:
        if (isprint(*itr)) {
          rVal.push_back(*itr);
        } else {
          rVal.push_back('\\');
          char buf[4] = {};
          const unsigned int v = static_cast<unsigned char>(*itr);
          snprintf(buf, 4, "%03d", v);
          rVal.append(buf);
        }
        break;
    }
    ++itr;
  }
  return rVal;
}

/**
 * Run all registered test cases.
 */
int runRegisteredTests() {
  registerTestCase(__FILE__, "check tests", checkTests);
  int errorCount = 0;
  const std::vector< TestInfo > &registry = getTestRegistry();
  const int sz = registry.size();
  std::cout << "1.." << sz << std::endl;
  for (std::vector< TestInfo >::const_iterator itr = registry.begin(); itr != registry.end(); ++itr) {
    const int currentId = std::distance(registry.begin(), itr) + 1;
    const float percent = 100.0f * ((float) currentId / (float) sz);
    try {
      itr->m_func();
      std::cout << "ok " << currentId << " (" << percent << "% done)" << std::endl;
    } catch (testing::TestAssertionException e) {
      std::cout << "not ok " << currentId << " Test failure at [" << e.file()
                << ":" << e.line() << " @ "
                << e.func() << "] > "
                << escape(e.what()) << std::endl;
      errorCount++;
    }
  }
  std::cout << "# Done testing." << std::endl;
  return errorCount;
}

} // namespace testing
