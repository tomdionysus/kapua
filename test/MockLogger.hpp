#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "Logger.hpp" // Include the Logger header

namespace KapuaTest {

class MockLogger : public Kapua::Logger {
 public:
  MOCK_METHOD(void, raw, (std::string log), (override));
  MOCK_METHOD(void, debug, (std::string log), (override));
  MOCK_METHOD(void, info, (std::string log), (override));
  MOCK_METHOD(void, warn, (std::string log), (override));
  MOCK_METHOD(void, error, (std::string log), (override));
};

} // namespace Kapua
