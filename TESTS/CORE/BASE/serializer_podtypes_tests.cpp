#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/BASE/serializer_podtypes.h>

class PodTestSink : public core::base::iBinarySerializerSink {
  public:
  PodTestSink() : m_pos(0) { memset(m_buffer, 0, ARRAY_LENGTH(m_buffer)); }

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

REGISTER_TEST_CASE(testVarInt) {
  for (u32 i = 0; i < (1 << 16); ++i) {
    PodTestSink sink;
    const VarInt expected(rand() * (rand() % 2 ? -1 : 1));
    sink << expected;
    sink.reset();
    VarInt result;
    sink >> result;
    TEST(testing::assertEquals(expected, result));
  }
}

REGISTER_TEST_CASE(testVarUInt) {
  for (u32 i = 0; i < (1 << 16); ++i) {
    PodTestSink sink;
    const VarUInt expected((u64) rand());
    sink << expected;
    sink.reset();
    VarUInt result;
    sink >> result;
    TEST(testing::assertEquals(expected, result));
  }
}
