#pragma once

#include "stdafx.h"
#include "variant.h"
#include "code_loc.h"
#include "state.h"
#include "operator.h"
#include "identifier.h"
#include "function_call_type.h"

struct Accu;

struct Node {
  Node(CodeLoc);
  CodeLoc codeLoc;
  enum CodegenStage { IMPORT, DECLARE_FUNCTIONS, DECLARE_TYPES, DEFINE };
  virtual void codegen(Accu&, CodegenStage) const {}
  virtual ~Node() {}
};

struct Expression : Node {
  using Node::Node;
  virtual SType getType(const State&) = 0;
  virtual nullable<SType> getDotOperatorType(const State& idContext, const State& callContext);
};

struct Constant : Expression {
  Constant(CodeLoc, SType, string value);
  virtual SType getType(const State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  SType type;
  string value;
};

struct EnumConstant : Expression {
  EnumConstant(CodeLoc, string enumName, string enumElement);
  virtual SType getType(const State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  string enumName;
  string enumElement;
};

struct Variable : Expression {
  Variable(CodeLoc, string);
  virtual SType getType(const State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  virtual nullable<SType> getDotOperatorType(const State& idContext, const State& callContext) override;
  string identifier;
};

struct BinaryExpression : Expression {
  BinaryExpression(CodeLoc, Operator, unique_ptr<Expression>, unique_ptr<Expression>);
  virtual SType getType(const State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  Operator op;
  unique_ptr<Expression> e1, e2;
};

struct UnaryExpression : Expression {
  UnaryExpression(CodeLoc, Operator, unique_ptr<Expression>);
  virtual SType getType(const State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  Operator op;
  unique_ptr<Expression> expr;
};

struct FunctionCall : Expression {
  FunctionCall(CodeLoc, IdentifierInfo);
  virtual SType getType(const State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  virtual nullable<SType> getDotOperatorType(const State& idContext, const State& callContext) override;
  IdentifierInfo identifier;
  optional<FunctionType> functionType;
  vector<unique_ptr<Expression>> arguments;
};

struct FunctionCallNamedArgs : Expression {
  FunctionCallNamedArgs(CodeLoc, IdentifierInfo);
  virtual SType getType(const State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  virtual nullable<SType> getDotOperatorType(const State& idContext, const State& callContext) override;
  IdentifierInfo identifier;
  struct Argument {
    CodeLoc codeLoc;
    string name;
    unique_ptr<Expression> expr;
  };
  optional<FunctionType> functionType;
  vector<Argument> arguments;
};

struct Statement : Node {
  using Node::Node;
  virtual void addToState(State&);
  virtual void check(State&) = 0;
  virtual bool hasReturnStatement(const State&) const;
  virtual void codegen(Accu&, CodegenStage) const = 0;
  enum class TopLevelAllowance {
    CANT,
    CAN,
    MUST
  };
  virtual TopLevelAllowance allowTopLevel() const { return TopLevelAllowance::CANT; }
};

struct VariableDeclaration : Statement {
  VariableDeclaration(CodeLoc, optional<IdentifierInfo> type, string identifier, unique_ptr<Expression> initExpr);
  optional<IdentifierInfo> type;
  nullable<SType> realType;
  string identifier;
  unique_ptr<Expression> initExpr;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
};

struct IfStatement : Statement {
  IfStatement(CodeLoc, unique_ptr<Expression> cond, unique_ptr<Statement> ifTrue,
      unique_ptr<Statement> ifFalse /* can be null*/);
  unique_ptr<Expression> cond;
  unique_ptr<Statement> ifTrue, ifFalse;
  virtual bool hasReturnStatement(const State&) const override;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
};

struct StatementBlock : Statement {
  using Statement::Statement;
  vector<unique_ptr<Statement>> elems;
  virtual bool hasReturnStatement(const State&) const override;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
};

struct ReturnStatement : Statement {
  using Statement::Statement;
  unique_ptr<Expression> expr;
  virtual bool hasReturnStatement(const State&) const override;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
};

struct ExpressionStatement : Statement {
  ExpressionStatement(unique_ptr<Expression>);
  unique_ptr<Expression> expr;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
};

struct ForLoopStatement : Statement {
  ForLoopStatement(CodeLoc l, unique_ptr<Statement> init, unique_ptr<Expression> cond, unique_ptr<Expression> iter,
      unique_ptr<Statement> body);
  unique_ptr<Statement> init;
  unique_ptr<Expression> cond;
  unique_ptr<Expression> iter;
  unique_ptr<Statement> body;
  virtual bool hasReturnStatement(const State&) const override;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
};

struct FunctionDefinition;

struct StructDefinition : Statement {
  StructDefinition(CodeLoc, string name);
  string name;
  struct Member {
    IdentifierInfo type;
    string name;
    CodeLoc codeLoc;
  };
  vector<Member> members;
  vector<unique_ptr<FunctionDefinition>> methods;
  vector<string> templateParams;
  nullable<SType> type;
  virtual void addToState(State&) override;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  virtual TopLevelAllowance allowTopLevel() const override { return TopLevelAllowance::MUST; }
  bool external = false;
  private:
  void generate(Accu&, bool import) const;
};

struct VariantDefinition : Statement {
  VariantDefinition(CodeLoc, string name);
  string name;
  struct Element {
    IdentifierInfo type;
    string name;
    CodeLoc codeLoc;
  };
  vector<Element> elements;
  vector<unique_ptr<FunctionDefinition>> methods;
  vector<string> templateParams;
  nullable<SType> type;
  virtual void addToState(State&) override;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  virtual TopLevelAllowance allowTopLevel() const override { return TopLevelAllowance::MUST; }

  private:
  void generate(Accu&, bool import) const;
};

struct EnumDefinition : Statement {
  EnumDefinition(CodeLoc, string name);
  string name;
  vector<string> elements;
  vector<unique_ptr<FunctionDefinition>> methods;
  virtual void addToState(State&) override;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  virtual TopLevelAllowance allowTopLevel() const override { return TopLevelAllowance::MUST; }

  private:
  void generate(Accu&, bool import) const;
};

struct SwitchStatement : Statement {
  SwitchStatement(CodeLoc, unique_ptr<Expression>);
  struct CaseElem {
    CodeLoc codeloc;
    optional<IdentifierInfo> type;
    string id;
    unique_ptr<StatementBlock> block;
    bool declareVar;
  };
  string subtypesPrefix;
  vector<CaseElem> caseElems;
  unique_ptr<StatementBlock> defaultBlock;
  unique_ptr<Expression> expr;
  enum { ENUM, VARIANT} type;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  virtual bool hasReturnStatement(const State&) const override;

  private:
  void checkVariant(State&, StructType, const string& typeName);
  void checkEnum(State&, EnumType, const string& typeName);
  void codegenEnum(Accu&) const;
  void codegenVariant(Accu&) const;
};

struct FunctionDefinition : Statement {
  FunctionDefinition(CodeLoc, IdentifierInfo returnType, string name);
  FunctionDefinition(CodeLoc, IdentifierInfo returnType, Operator);
  IdentifierInfo returnType;
  variant<string, Operator> nameOrOp;
  struct Parameter {
    CodeLoc codeLoc;
    IdentifierInfo type;
    string name;
  };
  vector<Parameter> parameters;
  unique_ptr<Statement> body;
  vector<string> templateParams;
  optional<FunctionType> functionType;
  virtual void check(State&) override;
  virtual void addToState(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  virtual TopLevelAllowance allowTopLevel() const override { return TopLevelAllowance::MUST; }
  void setFunctionType(const State&);
  void checkFunction(State&, bool templateStruct);
  void addSignature(Accu& accu, string structName) const;
};

struct EmbedStatement : Statement {
  EmbedStatement(CodeLoc, string value);
  string value;
  bool isPublic = false;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  virtual TopLevelAllowance allowTopLevel() const override;
  virtual bool hasReturnStatement(const State&) const override;
};

struct AST;

struct ImportStatement : Statement {
  ImportStatement(CodeLoc, string path, bool isPublic);
  string path;
  unique_ptr<AST> ast;
  bool isPublic;
  virtual void addToState(State&) override;
  virtual void check(State&) override;
  virtual void codegen(Accu&, CodegenStage) const override;
  virtual TopLevelAllowance allowTopLevel() const override { return TopLevelAllowance::MUST; }
};

struct AST {
  vector<unique_ptr<Statement>> elems;
};
