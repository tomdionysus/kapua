//
// Kapua Action classes
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

namespace Kapua {

enum ActionType : uint8_t {
  RequestPublicKey,
};

class Action {
public:
  ActionType type;
};

class ActionRequestPublicKey: public Action {
public:
  uint64_t node_id;

  ActionRequestPublicKey() {
    type = ActionType::RequestPublicKey;
  }
};

};  // namespace Kapua