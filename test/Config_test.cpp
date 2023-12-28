#include <gtest/gtest.h>

#include "Config.hpp"

// Demonstrate some basic assertions.
TEST(ConfigTest, BasicAssertions) {
  auto config = std::unique_ptr<Kapua::Config>(new Kapua::Config(nullptr,"test/fixtures/config_full.yaml"));

  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}