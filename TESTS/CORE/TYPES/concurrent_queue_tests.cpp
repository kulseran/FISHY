#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/TYPES/concurrent_queue.h>

#include <vector>

using core::types::ConcurrentQueue;

struct TestState {
  TestState(ConcurrentQueue< int > &queue, int count)
      : m_queue(queue), m_count(count) {}

  ConcurrentQueue< int > &m_queue;
  int m_count;
};

static void PushTester(TestState state) {
  for (int i = 0; i < state.m_count; ++i) {
    state.m_queue.push(i);
  }
}

static void PushNTester(TestState state) {
  std::vector< int > buffer;
  for (int i = 0; i < state.m_count; ++i) {
    buffer.push_back(i);
  }
  state.m_queue.pushN(buffer.begin(), buffer.end());
}

static void PopTester(TestState state) {
  for (int i = 0; i < state.m_count; ++i) {
    int val = -1;
    TEST(testing::assertTrue(state.m_queue.pop(val)));
    TEST(testing::assertEquals(val, i));
  }
}

static void PopNTester(TestState state) {
  std::vector< int > buffer;
  TEST(testing::assertTrue(state.m_queue.popN(state.m_count, buffer)));
}

REGISTER_TEST_CASE(testEnqueueFull) {
  ConcurrentQueue< int > queue(1);
  std::thread testThread(PushTester, TestState(queue, 2));
  while (queue.size() != 1) {
    std::this_thread::yield();
  }
  PopTester(TestState(queue, 1));
  testThread.join();
  queue.close();
}

REGISTER_TEST_CASE(testEnqueueNFull) {
  ConcurrentQueue< int > queue(2);
  std::thread testThread(PushNTester, TestState(queue, 4));
  while (queue.size() != 2) {
    std::this_thread::yield();
  }
  PopTester(TestState(queue, 2));
  testThread.join();
  queue.close();
}

REGISTER_TEST_CASE(testPopEmpty) {
  ConcurrentQueue< int > queue(1);
  std::thread testThread(PopTester, TestState(queue, 2));
  PushTester(TestState(queue, 2));
  testThread.join();
  queue.close();
}

REGISTER_TEST_CASE(testPopNEmpty) {
  ConcurrentQueue< int > queue(1);
  std::thread testThread(PopNTester, TestState(queue, 2));
  PushTester(TestState(queue, 2));
  testThread.join();
  queue.close();
}
