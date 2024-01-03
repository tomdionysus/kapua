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

// parse duration

TEST_F(ConfigTest, ParseDuration_Hours) {
  int64_t milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("1h", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 3600000);
}

TEST_F(ConfigTest, ParseDuration_Minutes) {
  int64_t milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("5m30s", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 330000);
}

TEST_F(ConfigTest, ParseDuration_Complex) {
  int64_t milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("1h27m16s", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 5236000);
}

TEST_F(ConfigTest, ParseDuration_Milliseconds) {
  int64_t milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("500u", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 500);
}

TEST_F(ConfigTest, ParseDuration_Fractional) {
  int64_t milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("0.5h", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 1800000);
}

TEST_F(ConfigTest, ParseDuration_Combined) {
  int64_t milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("1h0.5m", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::Success);
  EXPECT_EQ(milliseconds, 3630000);
}

TEST_F(ConfigTest, ParseDuration_InvalidFormat) {
  int64_t milliseconds;
  ConfigTestHelper::ParseResult result = config->parse_duration("1h2m3.4s5u", milliseconds);
  EXPECT_EQ(result, ConfigTestHelper::ParseResult::InvalidFormat);
}

TEST_F(ConfigTest, ParseDuration_InvalidUnit) {
  int64_t milliseconds;
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

// parse_hex_uint64

TEST_F(ConfigTest, ValidHexWith0xPrefix) {
  uint64_t value = 0;
  ConfigTestHelper::ParseResult result = config->parse_hex_uint64("0x1a2b3c4d5e6f7890", value);
  EXPECT_EQ(result, Config::ParseResult::Success);
  EXPECT_EQ(value, 0x1a2b3c4d5e6f7890ULL);
}

TEST_F(ConfigTest, ValidHexWithout0xPrefix) {
  uint64_t value = 0;
  ConfigTestHelper::ParseResult result = config->parse_hex_uint64("1a2b3c4d5e6f7890", value);
  EXPECT_EQ(result, Config::ParseResult::Success);
  EXPECT_EQ(value, 0x1a2b3c4d5e6f7890ULL);
}

TEST_F(ConfigTest, InvalidHexLength) {
  uint64_t value = 0;
  ConfigTestHelper::ParseResult result = config->parse_hex_uint64("1A2B3C4D5E6F7890", value);
  EXPECT_EQ(result, Config::ParseResult::Success);
  EXPECT_EQ(value, 0x1a2b3c4d5e6f7890ULL);
}

TEST_F(ConfigTest, InvalidHexCharacters) {
  uint64_t value = 0;
  ConfigTestHelper::ParseResult result = config->parse_hex_uint64("0x1a2b3c4d5e6f789012", value);
  EXPECT_EQ(result, Config::ParseResult::InvalidFormat);
}

TEST_F(ConfigTest, PrefixButNoData) {
  uint64_t value = 0;
  ConfigTestHelper::ParseResult result = config->parse_hex_uint64("0x", value);
  EXPECT_EQ(result, Config::ParseResult::InvalidFormat);
}

TEST_F(ConfigTest, InvalidTooShort) {
  uint64_t value = 0;
  ConfigTestHelper::ParseResult result = config->parse_hex_uint64("0x2871239", value);
  EXPECT_EQ(result, Config::ParseResult::InvalidFormat);
}

TEST_F(ConfigTest, EmptyString) {
  uint64_t value = 0;
  ConfigTestHelper::ParseResult result = config->parse_hex_uint64("", value);
  EXPECT_EQ(result, Config::ParseResult::InvalidFormat);
}

TEST_F(ConfigTest, InvalidHexPrefix) {
  uint64_t value = 0;
  ConfigTestHelper::ParseResult result = config->parse_hex_uint64("0y1a2b3c4d5e6f7890", value);
  EXPECT_EQ(result, Config::ParseResult::InvalidFormat);
}

}  // namespace KapuaTest
