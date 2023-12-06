//
// Kapua Node class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023 
//
#include "Node.hpp"

#include "Core.hpp"

using namespace std;

namespace Kapua {
Node ::Node(uint64_t id) { _id = id; }

Node ::~Node() {}
}  // namespace Kapua