/**
 * Unit test harness
 */
#include <iostream>

int main(int argc, char **argv) {
  std::clog << "# Running tests..." << std::endl;
  int errorCount = 0;
  std::clog << "# Done. With " << errorCount << " errors." << std::endl;

  return ((errorCount == 0) ? 0 : 1);
}