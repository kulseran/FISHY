#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/BASE/appstats.h>

using core::AppInfo;
using core::AppStat;

REGISTER_TEST_CASE(testAppstatGetSetReset) {
  AppStat myStat("teststat");
  TEST(testing::assertEquals(myStat.get(), 0));
  myStat.increment();
  TEST(testing::assertEquals(myStat.get(), 1));
  myStat.increment(19);
  TEST(testing::assertEquals(myStat.get(), 20));
  myStat.reset();
  TEST(testing::assertEquals(myStat.get(), 0));
}

REGISTER_TEST_CASE(testAppstatGetSetResetGlobal) {
  AppStat myStat("teststat");
  AppStat myStatRef("teststat");
  TEST(testing::assertEquals(myStatRef.get(), 0));
  myStat.increment();
  TEST(testing::assertEquals(myStatRef.get(), 1));
  myStat.increment(19);
  TEST(testing::assertEquals(myStatRef.get(), 20));
  myStat.reset();
  TEST(testing::assertEquals(myStatRef.get(), 0));
}

REGISTER_TEST_CASE(testAppinfoGetSetReset) {
  AppInfo myInfo("testinfo");
  TEST(testing::assertEquals(myInfo.get(), ""));
  const std::string infoStr = "more info";
  myInfo.set(infoStr);
  TEST(testing::assertEquals(myInfo.get(), infoStr));
  myInfo.reset();
  TEST(testing::assertEquals(myInfo.get(), ""));
}

REGISTER_TEST_CASE(testAppinfoGetSetResetGlobal) {
  AppInfo myInfo("testinfo");
  AppInfo myInfoRef("testinfo");
  TEST(testing::assertEquals(myInfoRef.get(), ""));
  const std::string infoStr = "more info";
  myInfo.set(infoStr);
  TEST(testing::assertEquals(myInfoRef.get(), infoStr));
  myInfo.reset();
  TEST(testing::assertEquals(myInfoRef.get(), ""));
}
