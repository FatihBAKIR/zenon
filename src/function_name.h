#pragma once

#include "stdafx.h"
#include "operator.h"

struct ConstructorId {
  ConstructorId(string name) : name(name) {}
  string name;
  bool operator < (ConstructorId o) const {
    return name < o.name;
  }
};
using FunctionName = variant<string, Operator, ConstructorId>;
using FunctionId = variant<string, Operator, SType>;
