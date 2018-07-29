#include "identifier.h"
#include "ast.h"

IdentifierInfo::IdentifierInfo(string n, CodeLoc l) : parts(1, IdentifierPart{n, {}}), codeLoc(l) {
  CHECK(!n.empty());
}

IdentifierInfo::IdentifierInfo(IdentifierInfo::IdentifierPart part, CodeLoc l) : parts(1, part), codeLoc(l) {
}

IdentifierInfo IdentifierInfo::getWithoutFirstPart() const {
  IdentifierInfo ret(*this);
  ret.parts.clear();
  for (int i = 1; i < parts.size(); ++i)
    ret.parts.push_back(parts[i]);
  return ret;
}

optional<string> IdentifierInfo::asBasicIdentifier() const {
  if (parts.size() > 1 || !parts[0].templateArguments.empty() || !pointerOrArray.empty())
    return none;
  else
    return parts[0].name;
}

string IdentifierInfo::toString() const {
  string ret;
  for (auto& part : parts) {
    if (!ret.empty())
      ret.append("::");
    ret.append(part.toString());
  }
  for (auto& elem : pointerOrArray)
    elem.visit(
        [&](PointerType type) {
          switch (type) {
            case MUTABLE:
              ret.append("*");
              break;
            case CONST:
              ret.append(" const*");
              break;
          }
        },
        [&](const ArraySize&) {
          ret.append("[expr]");
        }
    );
  return ret;
}

bool IdentifierInfo::operator ==(const IdentifierInfo& o) const {
  return parts == o.parts;
}

/*bool IdentifierInfo::operator ==(const IdentifierInfo& id) const {
  return name == id.name && namespaces == id.namespaces && templateParams == id.templateParams;
}*/

IdentifierInfo::IdentifierInfo() {

}

std::string IdentifierInfo::IdentifierPart::toString() const {
  string ret = name;
  if (!templateArguments.empty()) {
    auto paramsNames = transform(templateArguments, [](auto& e) { return e.toString(); });
    ret = ret + "<" + combine(paramsNames, ",") + ">";
  }
  return ret;
}

bool IdentifierInfo::IdentifierPart::operator == (const IdentifierPart& o) const {
  return name == o.name && templateArguments == o.templateArguments;
}
