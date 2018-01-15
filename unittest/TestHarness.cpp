#include "TestHarness.hpp"
#include "gtest/gtest.h"
#include "googletest/googletest/src/gtest-all.cc"

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

