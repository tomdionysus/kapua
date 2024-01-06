//
// Kapua Action classes
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

namespace Kapua {

enum ActionType : uint8_t {};

class Action {
 public:
  ActionType type;
};

};  // namespace Kapua