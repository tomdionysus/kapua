//
// Kapua core class
//
// Author: Tom Cully <mail@tomcully.com>
//

#pragma once

#include "Logger.hpp"

namespace Kapua {

class Core {
 public:
  Core(Logger& logger);
  ~Core();

 protected:
  Logger& _logger;
};

};  // namespace Kapua