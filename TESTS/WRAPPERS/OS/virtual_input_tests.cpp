#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <WRAPPERS/OS/virtual_input.h>

using wrappers::os::eAxisMap;
using wrappers::os::eDeviceId;
using wrappers::os::eKeyMap;
using wrappers::os::InputManager;

REGISTER_TEST_CASE(testInputManagerClear) {
  InputManager manager;
  TEST(testing::assertEquals(
      manager.getLastPressDevice(), eDeviceId::DEVICE_COUNT));
  TEST(testing::assertEquals(manager.getLastPressedKey(), eKeyMap::type_COUNT));
  TEST(
      testing::assertEquals(manager.getLastReleasedKey(), eKeyMap::type_COUNT));

  TEST(testing::assertEquals(
      manager.axisGetPos(eDeviceId::DEVICE_DEFAULT, eAxisMap::AXIS_MOUSE_X),
      0));
  TEST(testing::assertEquals(
      manager.keyIsDown(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A), false));
}

REGISTER_TEST_CASE(testInputManagerPressdetect) {
  InputManager manager;
  TEST(testing::assertEquals(
      manager.keyIsDown(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A), false));
  TEST(testing::assertEquals(
      manager.keyWasPressed(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A), false));
  TEST(testing::assertEquals(
      manager.keyWasReleased(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A),
      false));
  TEST(testing::assertEquals(manager.getLastPressedKey(), eKeyMap::type_COUNT));
  TEST(
      testing::assertEquals(manager.getLastReleasedKey(), eKeyMap::type_COUNT));

  manager.updateKey(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A, true);
  TEST(testing::assertEquals(
      manager.keyIsDown(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A), true));
  TEST(testing::assertEquals(
      manager.keyWasPressed(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A), true));
  TEST(testing::assertEquals(
      manager.keyWasReleased(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A),
      false));
  TEST(testing::assertEquals(manager.getLastPressedKey(), eKeyMap::KEY_A));
  TEST(
      testing::assertEquals(manager.getLastReleasedKey(), eKeyMap::type_COUNT));

  manager.updateKey(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A, false);
  TEST(testing::assertEquals(
      manager.keyIsDown(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A), false));
  TEST(testing::assertEquals(
      manager.keyWasPressed(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A), true));
  TEST(testing::assertEquals(
      manager.keyWasReleased(eDeviceId::DEVICE_DEFAULT, eKeyMap::KEY_A), true));
  TEST(testing::assertEquals(manager.getLastPressedKey(), eKeyMap::KEY_A));
  TEST(testing::assertEquals(manager.getLastReleasedKey(), eKeyMap::KEY_A));
}
