#include <gtest/gtest.h>
#include "Config.hpp"

class ConfigTestHelper : public Kapua::Config {
public:
    using Kapua::Config::parse_duration;

    ConfigTestHelper(Kapua::Logger* logger, std::string filename)
        : Kapua::Config(logger, filename) {}
};

class ConfigTest : public ::testing::Test {
protected:
    std::unique_ptr<ConfigTestHelper> config;

    void SetUp() override {
        // Create a ConfigTestHelper instance with a null logger for testing
        config = std::make_unique<ConfigTestHelper>(nullptr, "test/fixtures/config_full.yaml");
    }
};

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
