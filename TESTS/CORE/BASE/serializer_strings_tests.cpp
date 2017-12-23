#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/BASE/serializer_strings.h>

class StringTestSink : public core::base::iBinarySerializerSink {
  public:
  StringTestSink() : m_pos(0) { memset(m_buffer, 0, ARRAY_LENGTH(m_buffer)); }

  void reset() { m_pos = 0; }

  size_t write(const core::memory::ConstBlob &blob) override {
    TEST(testing::assertTrue(m_pos + blob.size() < ARRAY_LENGTH(m_buffer)));
    memcpy(m_buffer + m_pos, blob.data(), blob.size());
    m_pos += blob.size();
    return blob.size();
  }

  size_t read(core::memory::Blob &blob) override {
    TEST(testing::assertTrue(m_pos + blob.size() < ARRAY_LENGTH(m_buffer)));
    memcpy(blob.data(), m_buffer + m_pos, blob.size());
    m_pos += blob.size();
    return blob.size();
  }

  void seek(const size_t dist) override { m_pos += dist; }

  size_t avail() override { return ARRAY_LENGTH(m_buffer) - m_pos; }

  private:
  u8 m_buffer[1000];
  size_t m_pos;
};

REGISTER_TEST_CASE(testString) {
  const std::string expected = "hello world";
  std::string actual;
  StringTestSink sink;
  sink << expected;
  sink.reset();
  sink >> actual;
  TEST(testing::assertEquals(
      0, memcmp(actual.data(), expected.data(), expected.size())));
}
