#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/state_machine.h>
#include <CORE/types.h>

using core::util::iState;
using core::util::StateMachine;

class TestState1 : public iState {
  public:
  TestState1()
      : m_was_init(false),
        m_was_term(false),
        m_updates(0),
        m_enters(0),
        m_exits(0) {}

  void onInit() override { m_was_init = true; }
  void onTerm() override { m_was_term = true; }
  int update() override {
    m_updates++;
    return 0;
  }
  void onEnter() override { m_enters++; }
  void onExit() override { m_exits++; }
  const char *getDebugName() const override { return "TestState1"; }

  bool m_was_init;
  bool m_was_term;
  u32 m_updates;
  u32 m_enters;
  u32 m_exits;
};

class TestState2 : public iState {
  public:
  const char *getDebugName() const override { return "TestState2"; }
};

REGISTER_TEST_CASE(stateMachineTestAddUnique) {
  StateMachine machine;
  TestState1 testState1;

  TEST(testing::assertTrue(machine.addState(&testState1)));
  TEST(testing::assertFalse(machine.addState(&testState1)));
}

REGISTER_TEST_CASE(stateMachineTestInitTerm) {
  StateMachine machine;
  TestState1 testState1;

  TEST(testing::assertTrue(machine.addState(&testState1)));
  machine.init();
  TEST(testing::assertTrue(testState1.m_was_init));
  machine.term();
  TEST(testing::assertTrue(testState1.m_was_term));
}

REGISTER_TEST_CASE(stateMachineTestEnterUpdateExit) {
  StateMachine machine;
  TestState1 testState1;
  TestState1 testState2;
  TEST(testing::assertTrue(machine.addState(&testState1)));
  TEST(testing::assertTrue(machine.addState(&testState2)));
  machine.init();

  machine.requestTransition(
      fsengine::system::STATE_INVALID, testState1.getId());
  TEST(testing::assertEquals(0, machine.update()));
  TEST(testing::assertEquals(0, machine.update()));
  machine.requestTransition(testState1.getId(), testState2.getId());
  TEST(testing::assertEquals(0, machine.update()));

  TEST(testing::assertEquals(1, testState1.m_enters));
  TEST(testing::assertEquals(1, testState2.m_enters));
  TEST(testing::assertEquals(1, testState1.m_exits));
  TEST(testing::assertEquals(2, testState1.m_updates));
  TEST(testing::assertEquals(1, testState2.m_updates));

  machine.term();
}

REGISTER_TEST_CASE(sateMachineTestDefaultStateBehavior) {
  StateMachine machine;
  TestState2 testState1;
  TestState2 testState2;
  TEST(testing::assertTrue(machine.addState(&testState1)));
  TEST(testing::assertTrue(machine.addState(&testState2)));
  machine.init();

  machine.requestTransition(
      fsengine::system::STATE_INVALID, testState1.getId());
  TEST(testing::assertEquals(0, machine.update()));
  TEST(testing::assertEquals(0, machine.update()));
  machine.requestTransition(testState1.getId(), testState2.getId());
  TEST(testing::assertEquals(0, machine.update()));

  machine.term();
}
