#include "Config.hpp"

#include <gtest/gtest.h>

#include "Logger.hpp"

using namespace Kapua;

class ConfigTestHelper : public Config {
 public:
  using Config::parse_bool;
  using Config::parse_duration;
  using Config::parse_log_level;

  ConfigTestHelper(Logger* logger, std::string filename) : Config(logger, filename) {}
};

class ConfigTest : public ::testing::Test {
 protected:
  std::unique_ptr<ConfigTestHelper> config;

  void SetUp() override {
    // Create a ConfigTestHelper instance with a null logger for testing
    config = std::make_unique<ConfigTestHelper>(nullptr, "test/fixtures/config_full.yaml");
  }
};

// parse duration

TEST_F(ConfigTest, ParseDuration_Hours) {
  long long milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("1h", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 3600000);
}

TEST_F(ConfigTest, ParseDuration_Minutes) {
  long long milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("5m30s", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 330000);
}

TEST_F(ConfigTest, ParseDuration_Complex) {
  long long milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("1h27m16s", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 5236000);
}

TEST_F(ConfigTest, ParseDuration_Milliseconds) {
  long long milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("500u", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 500);
}

TEST_F(ConfigTest, ParseDuration_Fractional) {
  long long milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("0.5h", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 1800000);
}

TEST_F(ConfigTest, ParseDuration_Combined) {
  long long milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("1h0.5m", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 3630000);
}

TEST_F(ConfigTest, ParseDuration_InvalidFormat) {
  long long milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("1h2m3.4s5u", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::InvalidFormat);
}

TEST_F(ConfigTest, ParseDuration_InvalidUnit) {
  long long milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("1k", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::InvalidUnit);
}

// parse_log_level

TEST_F(ConfigTest, ParseLogLevel_Error) {
  LogLevel_t level;
  ConfigTestHelper::ParseResult result = config->parse_log_level("error", &level);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(level, LOG_LEVEL_ERROR);
}

TEST_F(ConfigTest, ParseLogLevel_Warn) {
  LogLevel_t level;
  ConfigTestHelper::ParseResult result = config->parse_log_level("warn", &level);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(level, LOG_LEVEL_WARN);
}

TEST_F(ConfigTest, ParseLogLevel_Warning) {
  LogLevel_t level;
  ConfigTestHelper::ParseResult result = config->parse_log_level("warning", &level);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(level, LOG_LEVEL_WARN);
}

TEST_F(ConfigTest, ParseLogLevel_Info) {
  LogLevel_t level;
  ConfigTestHelper::ParseResult result = config->parse_log_level("info", &level);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(level, LOG_LEVEL_INFO);
}

TEST_F(ConfigTest, ParseLogLevel_Debug) {
  LogLevel_t level;
  ConfigTestHelper::ParseResult result = config->parse_log_level("debug", &level);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(level, LOG_LEVEL_DEBUG);
}

TEST_F(ConfigTest, ParseLogLevel_MixedCase) {
  LogLevel_t level;
  ConfigTestHelper::ParseResult result = config->parse_log_level("WaRn", &level);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(level, LOG_LEVEL_WARN);
}

TEST_F(ConfigTest, ParseLogLevel_InvalidFormat) {
  LogLevel_t level;
  ConfigTestHelper::ParseResult result = config->parse_log_level("invalid", &level);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::InvalidFormat);
}

TEST_F(ConfigTest, ParseLogLevel_EmptyString) {
  LogLevel_t level;
  ConfigTestHelper::ParseResult result = config->parse_log_level("", &level);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::InvalidFormat);
}

// parse_bool

TEST_F(ConfigTest, ParseBool_TrueLowercase) {
  bool result;
  ConfigTestHelper::ParseResult parseResult = config->parse_bool("true", &result);
  EXPECT_EQ(parseResult, ConfigTestHelper::ParseResult::Success);
  EXPECT_TRUE(result);
}

TEST_F(ConfigTest, ParseBool_TrueUppercase) {
  bool result;
  ConfigTestHelper::ParseResult parseResult = config->parse_bool("TRUE", &result);
  EXPECT_EQ(parseResult, ConfigTestHelper::ParseResult::Success);
  EXPECT_TRUE(result);
}

TEST_F(ConfigTest, ParseBool_T) {
  bool result;
  ConfigTestHelper::ParseResult parseResult = config->parse_bool("t", &result);
  EXPECT_EQ(parseResult, ConfigTestHelper::ParseResult::Success);
  EXPECT_TRUE(result);
}

TEST_F(ConfigTest, ParseBool_YesLowercase) {
  bool result;
  ConfigTestHelper::ParseResult parseResult = config->parse_bool("yes", &result);
  EXPECT_EQ(parseResult, ConfigTestHelper::ParseResult::Success);
  EXPECT_TRUE(result);
}

TEST_F(ConfigTest, ParseBool_FalseLowercase) {
  bool result;
  ConfigTestHelper::ParseResult parseResult = config->parse_bool("false", &result);
  EXPECT_EQ(parseResult, ConfigTestHelper::ParseResult::Success);
  EXPECT_FALSE(result);
}

TEST_F(ConfigTest, ParseBool_FalseUppercase) {
  bool result;
  ConfigTestHelper::ParseResult parseResult = config->parse_bool("FALSE", &result);
  EXPECT_EQ(parseResult, ConfigTestHelper::ParseResult::Success);
  EXPECT_FALSE(result);
}

TEST_F(ConfigTest, ParseBool_F) {
  bool result;
  ConfigTestHelper::ParseResult parseResult = config->parse_bool("f", &result);
  EXPECT_EQ(parseResult, ConfigTestHelper::ParseResult::Success);
  EXPECT_FALSE(result);
}

TEST_F(ConfigTest, ParseBool_NoLowercase) {
  bool result;
  ConfigTestHelper::ParseResult parseResult = config->parse_bool("no", &result);
  EXPECT_EQ(parseResult, ConfigTestHelper::ParseResult::Success);
  EXPECT_FALSE(result);
}

TEST_F(ConfigTest, ParseBool_InvalidFormat) {
  bool result;
  ConfigTestHelper::ParseResult parseResult = config->parse_bool("invalid", &result);
  EXPECT_EQ(parseResult, ConfigTestHelper::ParseResult::InvalidFormat);
}