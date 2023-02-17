#pragma once

#include "CParser.h"
#include "asg.hpp"
#include <unordered_map>

namespace asg {

using ast = antlr_c::CParser;

class Ast2Asg
{
public:
  Obj::Mgr _mgr;

public:
  TranslationUnit operator()(ast::TranslationUnitContext* ctx);

private:
  struct LocalDecls : public std::unordered_map<std::string, Decl*>
  {
    Ast2Asg& _self;
    LocalDecls* _prev;

    LocalDecls(Ast2Asg& self)
      : _self(self)
      , _prev(self._localDecls)
    {
      _self._localDecls = this;
    }

    ~LocalDecls() { _self._localDecls = _prev; }

    Decl* resolve(const std::string& name);
  };

  LocalDecls* _localDecls{ nullptr };

  struct CurrentLoop
  {
    Ast2Asg& _self;
    Stmt* _prev;

    CurrentLoop(Ast2Asg& self, Stmt* loop)
      : _self(self)
      , _prev(self._currentLoop)
    {
      _self._currentLoop = loop;
    }

    ~CurrentLoop() { _self._currentLoop = _prev; }
  };

  Stmt* _currentLoop{ nullptr };

  FunctionDecl* _currentFunc{ nullptr };

private:
  //============================================================================
  // 类型
  //============================================================================

  Type::Specs operator()(ast::DeclarationSpecifiersContext* ctx);

  Type::Specs operator()(ast::DeclarationSpecifiers2Context* ctx);

  std::pair<TypeExpr*, std::string> operator()(ast::DeclaratorContext* ctx);

  std::pair<TypeExpr*, std::string> operator()(
    ast::DirectDeclaratorContext* ctx);

  TypeExpr* operator()(ast::AbstractDeclaratorContext* ctx);

  TypeExpr* operator()(ast::DirectAbstractDeclaratorContext* ctx);

  //============================================================================
  // 表达式
  //============================================================================

  Expr* operator()(ast::ExpressionContext* ctx);

  Expr* operator()(ast::AssignmentExpressionContext* ctx);

  Expr* operator()(ast::LogicalOrExpressionContext* ctx);

  Expr* operator()(ast::LogicalAndExpressionContext* ctx);

  Expr* operator()(ast::EqualityExpressionContext* ctx);

  Expr* operator()(ast::RelationalExpressionContext* ctx);

  Expr* operator()(ast::AdditiveExpressionContext* ctx);

  Expr* operator()(ast::MultiplicativeExpressionContext* ctx);

  Expr* operator()(ast::UnaryExpressionContext* ctx);

  Expr* operator()(ast::PostfixExpressionContext* ctx);

  Expr* operator()(ast::PrimaryExpressionContext* ctx);

  Expr* operator()(ast::InitializerContext* ctx);

  //============================================================================
  // 语句
  //============================================================================

  Stmt* operator()(ast::StatementContext* ctx);

  CompoundStmt* operator()(ast::CompoundStatementContext* ctx);

  Stmt* operator()(ast::ExpressionStatementContext* ctx);

  Stmt* operator()(ast::SelectionStatementContext* ctx);

  Stmt* operator()(ast::IterationStatementContext* ctx);

  Stmt* operator()(ast::JumpStatementContext* ctx);

  //============================================================================
  // 声明
  //============================================================================

  std::vector<Decl*> operator()(ast::DeclarationContext* ctx);

  FunctionDecl* operator()(ast::FunctionDefinitionContext* ctx);

  VarDecl* operator()(ast::InitDeclaratorContext* ctx, Type::Specs specs);

  VarDecl* operator()(ast::ParameterDeclarationContext* ctx);

private:
  template<typename T, typename... Args>
  T& make(Args... args)
  {
    return _mgr.make<T>(args...);
  }
};

}
