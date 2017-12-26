#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/VFS/FILTERS/filter_passthrough.h>

using vfs::filters::passthrough;

REGISTER_TEST_CASE(testFilterRead) {
  std::stringstream buffer;
  passthrough filter;
}
