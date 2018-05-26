#pragma once

#include "stdafx.h"
#include "hashing.h"
#include "util.h"
#include "token.h"

struct IdentifierInfo {
  struct IdentifierPart {
    string name;
    vector<IdentifierInfo> templateArguments;
    string toString() const;
    bool operator == (const IdentifierPart&) const;
    HASH_ALL(name, templateArguments)
  };
  explicit IdentifierInfo(string name);
  IdentifierInfo(IdentifierPart);
  //IdentifierInfo(vector<string> namespaces, string name, vector<IdentifierInfo> templateParams = {});
  static IdentifierInfo parseFrom(Tokens&, bool allowPointer);
  IdentifierInfo getWithoutFirstPart() const;
  optional<string> asBasicIdentifier() const;
  vector<IdentifierPart> parts;
  enum PointerType {
    MUTABLE,
    CONST
  };
  optional<PointerType> pointerType;
  CodeLoc codeLoc;
  string toString() const;
  bool operator == (const IdentifierInfo&) const;
  HASH_ALL(parts)
  private:
  IdentifierInfo();
};
