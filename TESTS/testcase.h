/**
 * testcase.h
 *
 * Core class for generate a test suit
 */
#ifndef FISHY_TEST_CASE_H
#define FISHY_TEST_CASE_H

#ifndef TESTING
#  error may only be included in tests
#endif

#include <string>

namespace testing {

/**
 * Delegate function to be run as a test
 */
typedef void (*tTestFunc)(void);

/**
 * Register a function to be run in the test suite
 */
void registerTestCase(const char *file, const char *name, tTestFunc func);

/**
 * Helper utility to register a test case.
 */
class StaticRegister {
  public:
  StaticRegister(const char *file, const char *name, tTestFunc func) {
    registerTestCase(file, name, func);
  }
};

/**
 * Run all registered tests
 */
int runRegisteredTests();

} // namespace testing

#  ifndef UNIQUE_MACRO_NAME
#    define UNIQUE_MACRO_NAME MAKE_NAME(__LINE__, __COUNTER__)
#    define MAKE_NAME(line, counter) MAKE_NAME2(line, counter)
#    define MAKE_NAME2(line, counter) cta_failurecond_##line##counter
#  endif

#  define REGISTER_TEST_CASE(f_test_name)                     \
    void f_test_name(void);                                   \
    static testing::StaticRegister UNIQUE_MACRO_NAME(         \
        __FILE__, __FILE__##":"## #f_test_name, f_test_name); \
    void f_test_name(void)

#endif
