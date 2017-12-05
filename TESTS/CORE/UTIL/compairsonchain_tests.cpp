#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/comparisonchain.h>
#include <CORE/types.h>

#include <vector>

using core::util::ComparisonChain;

REGISTER_TEST_CASE(testComparisonBooeanEqual) {
  bool ret = ComparisonChain().andOne(5, 5).buildEqual();
  TEST(testing::assertEquals(ret, true));
}

REGISTER_TEST_CASE(testComparisonBooeanLt) {
  bool ret = ComparisonChain().andOne(4, 5).buildLessThan();
  TEST(testing::assertEquals(ret, true));
  ret = ComparisonChain().andOne(5, 4).buildLessThan();
  TEST(testing::assertEquals(ret, false));
}

REGISTER_TEST_CASE(testComparisonBooeanGt) {
  bool ret = ComparisonChain().andOne(7, 5).buildGreaterThan();
  TEST(testing::assertEquals(ret, true));
  ret = ComparisonChain().andOne(5, 7).buildGreaterThan();
  TEST(testing::assertEquals(ret, false));
}

REGISTER_TEST_CASE(testComparisonInteger) {
  int ret1 = ComparisonChain().andOne(5, 5).buildInt();
  TEST(testing::assertEquals(ret1, 0));

  int ret2 = ComparisonChain().andOne(5, 6).buildInt();
  TEST(testing::assertEquals(ret2, -1));

  int ret3 = ComparisonChain().andOne(5, 4).buildInt();
  TEST(testing::assertEquals(ret3, 1));
}

REGISTER_TEST_CASE(testComparisonAnd) {
  int ret1 = ComparisonChain().andOne(5, 5).andOne(5, 6).buildInt();
  TEST(testing::assertEquals(ret1, -1));
}

REGISTER_TEST_CASE(testComparisonOrAfterEqual) {
  int ret1 = ComparisonChain().andOne(5, 5).orOne(5, 6).buildInt();
  TEST(testing::assertEquals(ret1, 0));
}

REGISTER_TEST_CASE(testComparisonOrAfterNotEqual) {
  int ret1 = ComparisonChain().andOne(5, 6).orOne(5, 5).buildInt();
  TEST(testing::assertEquals(ret1, 0));
}

REGISTER_TEST_CASE(testComparisonAndAll) {
  std::vector< int > list1;
  list1.push_back(1);
  list1.push_back(2);
  list1.push_back(3);

  std::vector< int > list2;
  list2.push_back(1);
  list2.push_back(2);
  list2.push_back(3);

  int ret1 = ComparisonChain()
                 .andAll(list1.begin(), list1.end(), list2.begin(), list2.end())
                 .buildInt();
  TEST(testing::assertEquals(ret1, 0));

  list2.push_back(4);
  int ret2 = ComparisonChain()
                 .andAll(list1.begin(), list1.end(), list2.begin(), list2.end())
                 .buildInt();
  TEST(testing::assertEquals(ret2, -1));

  list1.push_back(4);
  list1.push_back(5);
  int ret3 = ComparisonChain()
                 .andAll(list1.begin(), list1.end(), list2.begin(), list2.end())
                 .buildInt();
  TEST(testing::assertEquals(ret3, 1));

  list2.push_back(4);
  int ret4 = ComparisonChain()
                 .andAll(list1.begin(), list1.end(), list2.begin(), list2.end())
                 .buildInt();
  TEST(testing::assertEquals(ret4, 1));
}

REGISTER_TEST_CASE(testComparisonOrAny) {
  std::vector< int > list1;
  list1.push_back(1);
  list1.push_back(2);
  list1.push_back(3);

  std::vector< int > list2;
  list2.push_back(1);
  list2.push_back(2);
  list2.push_back(3);

  int ret1 = ComparisonChain()
                 .orAny(list1.begin(), list1.end(), list2.begin(), list2.end())
                 .buildInt();
  TEST(testing::assertEquals(ret1, 0));

  list2.push_back(4);
  int ret2 = ComparisonChain()
                 .orAny(list1.begin(), list1.end(), list2.begin(), list2.end())
                 .buildInt();
  TEST(testing::assertEquals(ret2, -1));

  list1.push_back(4);
  list1.push_back(5);
  int ret3 = ComparisonChain()
                 .orAny(list1.begin(), list1.end(), list2.begin(), list2.end())
                 .buildInt();
  TEST(testing::assertEquals(ret3, 1));

  list2.push_back(4);
  int ret4 = ComparisonChain()
                 .orAny(list1.begin(), list1.end(), list2.begin(), list2.end())
                 .buildInt();
  TEST(testing::assertEquals(ret4, 0));
}
