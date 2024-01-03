#include "Config.hpp"

#include <gtest/gtest.h>

#include "MockLogger.hpp"

using namespace Kapua;

namespace KapuaTest {

class ConfigTestHelper : public Config {
 public:
  using Config::parse_bool;
  using Config::parse_duration;
  using Config::parse_hex_uint64;
  using Config::parse_log_level;

  ConfigTestHelper(Logger* logger) : Config(logger) {}
};

class ConfigTest : public ::testing::Test {
 protected:
  std::unique_ptr<MockLogger> logger;
  std::unique_ptr<ConfigTestHelper> config;

  void SetUp() override {
    logger = std::make_unique<MockLogger>();
    config = std::make_unique<ConfigTestHelper>(logger.get());
  }
};

}  // namespace KapuaTest
