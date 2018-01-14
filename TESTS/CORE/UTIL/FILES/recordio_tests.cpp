#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/FILES/recordio.h>

using core::memory::Blob;
using core::memory::ConstBlob;
using core::util::files::InRecordIOStream;
using core::util::files::OutRecordIOStream;

REGISTER_TEST_CASE(testRecordReadWrite) {
  const std::string expected[] = {"Hello World", "Goodbye World"};

  std::stringstream buffer;
  OutRecordIOStream ofile(buffer);
  TEST(testing::assertEquals(
      ofile.appendRecord(ConstBlob(expected[0])).getStatus(), Status::OK));
  TEST(testing::assertEquals(
      ofile.appendRecord(ConstBlob(expected[1])).getStatus(), Status::OK));

  InRecordIOStream ifile(buffer);
  TEST(testing::assertEquals(ifile.skipForward(1).getStatus(), Status::OK));
  {
    std::string recordBuffer;
    recordBuffer.resize(64);
    Blob record(recordBuffer);
    TEST(testing::assertEquals(
        ifile.readNextRecord(record).getStatus(), Status::OK));
    TEST(testing::assertTrue(
        recordBuffer.substr(0, expected[1].length()) == expected[1]));
  }
  {
    std::string recordBuffer;
    recordBuffer.resize(64);
    Blob record(recordBuffer);
    TEST(testing::assertEquals(
        ifile.readNextRecord(record).getStatus(), Status::NOT_FOUND));
  }
  TEST(testing::assertEquals(
      ifile.skipForward(1).getStatus(), Status::NOT_FOUND));
}
