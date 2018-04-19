#pragma once
#include "stdafx.h"
#include "optional.h"
#include "variant.h"
#include "identifier.h"

struct IdentifierInfo;
struct Type;
struct FunctionType;
class Context;
using SContext = shared_ptr<Context>;
using SConstContext = shared_ptr<const Context>;

class Context : public owned_object<Context> {
  public:
  static Context withParent(const Context&);
  static Context withParent(vector<Context*>);
  Context();
  Context(const Context&) = delete;
  Context(Context&&) = default;
  void deepCopyFrom(const Context&);
  nullable<SType> getTypeOfVariable(const string&) const;
  void addVariable(const string& ident, SType);
  const vector<string>& getBottomLevelVariables() const;
  void replace(SType from, SType to);
  nullable<SType> getReturnType() const;
  void setReturnType(SType);
  void addType(const string& name, SType);
  nullable<SType> getTypeFromString(IdentifierInfo) const;
  void addFunction(variant<string, Operator> nameOrOp, FunctionType);
  vector<string> getFunctionParamNames(CodeLoc, IdentifierInfo) const;
  FunctionType getFunctionTemplate(CodeLoc, IdentifierInfo) const;
  FunctionType instantiateFunctionTemplate(CodeLoc, FunctionType, IdentifierInfo, vector<SType> argTypes, vector<CodeLoc> argLoc) const;
  optional<FunctionType> getOperatorType(Operator) const;
  void pushImport(const string& name);
  void popImport();
  const vector<string>& getImports() const;
  const vector<string>& getAllImports() const;
  void checkNameConflict(CodeLoc loc, const string& name, const string& type) const;
  vector<SType> getTypeList(const vector<IdentifierInfo>&) const;

  private:
  nullable<SType> getType(const string&) const;
  const FunctionType* getFunction(variant<string, Operator>) const;
  nullable<SType> getVariable(const string&) const;

  struct State : public owned_object<State> {
    map<string, SType> vars;
    vector<string> varsList;
    map<string, SType> types;
    map<string, FunctionType> functions;
    map<Operator, FunctionType> operators;
    nullable<SType> returnType;
    vector<string> imports;
    vector<string> allImports;
  };
  vector<shared_ptr<const State>> parentStates;
  shared_ptr<State> state;
  vector<shared_ptr<const Context::State> > getReversedStates() const;
  const State& getTopState() const;
};