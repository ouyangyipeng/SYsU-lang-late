#include "asg.hpp"
#include "Err.hpp"

#define self (*this)

namespace asg {

//==============================================================================
// Ast2Asg
//==============================================================================

Decl*
Ast2Asg::LocalDecls::resolve(const std::string& name)
{
  auto iter = find(name);
  if (iter != end())
    return iter->second;

  if (_prev == nullptr)
    throw err::Lit("undeclared identifier.");

  return _prev->resolve(name);
}

TranslationUnit
Ast2Asg::operator()(ast::TranslationUnitContext* ctx)
{
  TranslationUnit ret;
  if (ctx == nullptr)
    return ret;

  LocalDecls localDecls(self);

  for (auto&& i : ctx->externalDeclaration()) {
    if (auto p = i->declaration()) {
      auto decls = self(p);
      ret.insert(ret.end(),
                 std::make_move_iterator(decls.begin()),
                 std::make_move_iterator(decls.end()));
    }

    else if (auto p = i->functionDefinition()) {
      auto funcDecl = self(p);
      ret.push_back(funcDecl);

      // 添加到声明表
      localDecls[funcDecl->name] = funcDecl;
    }

    else if (auto p = i->Semi())
      ret.push_back(&make<Decl>());

    else
      assert(false);
  }

  return ret;
}

/// 类型

Type::Specs
Ast2Asg::operator()(ast::DeclarationSpecifiersContext* ctx)
{
  Type::Specs ret;

  for (auto&& i : ctx->declarationSpecifier()) {
    if (auto p = i->typeSpecifier()) {
      if (p->Long()) {
        if (ret.base == Type::Specs::kINVALID)
          ret.base = Type::Specs::kLong;
        else if (ret.base == Type::Specs::kLong)
          ret.base = Type::Specs::kLongLong;
      }

      else if (ret.base == Type::Specs::kINVALID) {
        if (p->Void())
          ret.base = Type::Specs::kVoid;
        else if (p->Char())
          ret.base = Type::Specs::kChar;
        else if (p->Int())
          ret.base = Type::Specs::kInt;
        else
          assert(false);
      }

      else
        throw err::Lit("invalid type specifier combination.");
    }

    else if (auto p = i->typeQualifier()) {
      if (p->Const())
        ret.isConst = true;
      else
        assert(false);
    }

    else
      assert(false);
  }

  return ret;
}

Type::Specs
Ast2Asg::operator()(ast::DeclarationSpecifiers2Context* ctx)
{
  Type::Specs ret;

  for (auto&& i : ctx->declarationSpecifier()) {
    if (auto p = i->typeSpecifier()) {
      if (p->Long()) {
        if (ret.base == Type::Specs::kINVALID)
          ret.base = Type::Specs::kLong;
        else if (ret.base == Type::Specs::kLong)
          ret.base = Type::Specs::kLongLong;
      }

      else if (ret.base == Type::Specs::kINVALID) {
        if (p->Void())
          ret.base = Type::Specs::kVoid;
        else if (p->Char())
          ret.base = Type::Specs::kChar;
        else if (p->Int())
          ret.base = Type::Specs::kInt;
        else
          assert(false);
      }

      throw err::Lit("invalid type specifier combination.");
    }

    else if (auto p = i->typeQualifier()) {
      if (p->Const())
        ret.isConst = true;
      else
        assert(false);
    }

    else
      assert(false);
  }

  return ret;
}

std::pair<TypeExpr*, std::string>
Ast2Asg::operator()(ast::DeclaratorContext* ctx)
{
  if (ctx->pointer()) {
    auto& ret = make<PointerType>();
    auto [sub, name] = self(ctx->directDeclarator());
    ret.sub = sub;
    return { &ret, std::move(name) };
  }

  return self(ctx->directDeclarator());
}

std::pair<TypeExpr*, std::string>
Ast2Asg::operator()(ast::DirectDeclaratorContext* ctx)
{
  if (auto p = ctx->Identifier())
    return { nullptr, p->getText() };

  if (auto p = ctx->declarator())
    return self(p);

  auto [sub, name] = self(ctx->directDeclarator());

  if (auto p = ctx->assignmentExpression()) {
    auto& ret = make<ArrayType>();
    ret.sub = sub;
    ret.len = self(p);
    return { &ret, std::move(name) };
  }

  if (auto p = ctx->parameterTypeList()) {
    auto& ret = make<FunctionType>();
    ret.sub = sub;
    for (auto&& i : p->parameterList()->parameterDeclaration())
      ret.params.push_back(self(i)->type);
    return { &ret, std::move(name) };
  }

  return { sub, std::move(name) };
}

TypeExpr*
Ast2Asg::operator()(ast::AbstractDeclaratorContext* ctx)
{
  if (ctx->pointer()) {
    auto& ret = make<PointerType>();

    if (auto p = ctx->directAbstractDeclarator())
      ret.sub = self(p);
    else
      ret.sub = nullptr;

    return &ret;
  }

  return self(ctx->directAbstractDeclarator());
}

TypeExpr*
Ast2Asg::operator()(ast::DirectAbstractDeclaratorContext* ctx)
{
  if (auto p = ctx->abstractDeclarator())
    return self(p);

  TypeExpr* ret;

  if (ctx->LeftBracket()) {
    auto& arrayType = make<ArrayType>();
    if (auto p = ctx->assignmentExpression())
      arrayType.len = self(p);
    ret = &arrayType;
  }

  else if (ctx->LeftParen()) {
    auto& funcType = make<FunctionType>();
    if (auto p = ctx->parameterTypeList()) {
      for (auto&& i : p->parameterList()->parameterDeclaration())
        funcType.params.push_back(self(i)->type);
    }
    ret = &funcType;
  }

  else
    assert(false);

  if (auto p = ctx->directAbstractDeclarator())
    ret->sub = self(p);

  return ret;
}

/// 表达式

Expr*
Ast2Asg::operator()(ast::ExpressionContext* ctx)
{
  auto list = ctx->assignmentExpression();
  if (list.size() == 1)
    return self(list[0]);

  auto& ret = make<BinaryExpr>();
  ret.op = ret.kComma;
  ret.lft = self(list[0]);

  auto node = &ret;
  for (unsigned i = 1; i < list.size() - 1; ++i) {
    auto& rht = make<BinaryExpr>();
    rht.op = rht.kComma;
    rht.lft = self(list[i]);
    node->rht = &rht;
    node = &rht;
  }

  node->rht = self(list[list.size() - 1]);

  return &ret;
}

Expr*
Ast2Asg::operator()(ast::AssignmentExpressionContext* ctx)
{
  if (auto p = ctx->logicalOrExpression())
    return self(p);

  auto& ret = make<BinaryExpr>();
  ret.op = ret.kAssign;
  ret.lft = self(ctx->unaryExpression());
  ret.rht = self(ctx->assignmentExpression());
  return &ret;
}

Expr*
Ast2Asg::operator()(ast::LogicalOrExpressionContext* ctx)
{
  auto list = ctx->logicalAndExpression();
  if (list.size() == 1)
    return self(list[0]);

  auto& ret = make<BinaryExpr>();
  ret.op = ret.kOr;
  ret.lft = self(list[0]);

  auto node = &ret;
  for (unsigned i = 1; i < list.size() - 1; ++i) {
    auto& rht = make<BinaryExpr>();
    rht.op = rht.kOr;
    rht.lft = self(list[i]);
    node->rht = &rht;
    node = &rht;
  }

  node->rht = self(list[list.size() - 1]);

  return &ret;
}

Expr*
Ast2Asg::operator()(ast::LogicalAndExpressionContext* ctx)
{
  auto list = ctx->equalityExpression();
  if (list.size() == 1)
    return self(list[0]);

  auto& ret = make<BinaryExpr>();
  ret.op = ret.kOr;
  ret.lft = self(list[0]);

  auto node = &ret;
  for (unsigned i = 1; i < list.size() - 1; ++i) {
    auto& rht = make<BinaryExpr>();
    rht.op = rht.kOr;
    rht.lft = self(list[i]);
    node->rht = &rht;
    node = &rht;
  }

  node->rht = self(list[list.size() - 1]);

  return &ret;
}

Expr*
Ast2Asg::operator()(ast::EqualityExpressionContext* ctx)
{
  auto& children = ctx->children;

  if (children.size() == 1)
    return self(dynamic_cast<ast::RelationalExpressionContext*>(children[0]));

  auto& ret = make<BinaryExpr>();
  ret.lft = self(dynamic_cast<ast::RelationalExpressionContext*>(children[0]));

  auto node = &ret;
  unsigned i = 1;
  while (true) {
    switch (dynamic_cast<antlr4::tree::TerminalNode*>(children[i])
              ->getSymbol()
              ->getType()) {
      case ast::Equal:
        node->op = node->kEq;
        break;

      case ast::NotEqual:
        node->op = node->kNe;
        break;

      default:
        assert(false);
    }

    if (i >= children.size() - 2)
      break;

    auto& rht = make<BinaryExpr>();
    rht.lft =
      self(dynamic_cast<ast::RelationalExpressionContext*>(children[i + 1]));
    node->rht = &rht;
    node = &rht;

    i += 2;
  }

  node->rht =
    self(dynamic_cast<ast::RelationalExpressionContext*>(children[i + 1]));

  return &ret;
}

Expr*
Ast2Asg::operator()(ast::RelationalExpressionContext* ctx)
{
  auto& children = ctx->children;

  if (children.size() == 1)
    return self(dynamic_cast<ast::AdditiveExpressionContext*>(children[0]));

  auto& ret = make<BinaryExpr>();
  ret.lft = self(dynamic_cast<ast::AdditiveExpressionContext*>(children[0]));

  auto node = &ret;
  unsigned i = 1;
  while (true) {
    switch (dynamic_cast<antlr4::tree::TerminalNode*>(children[i])
              ->getSymbol()
              ->getType()) {
      case ast::Less:
        node->op = node->kLt;
        break;

      case ast::LessEqual:
        node->op = node->kLe;
        break;

      case ast::Greater:
        node->op = node->kGt;
        break;

      case ast::GreaterEqual:
        node->op = node->kGe;
        break;

      default:
        assert(false);
    }

    if (i >= children.size() - 2)
      break;

    auto& rht = make<BinaryExpr>();
    rht.lft =
      self(dynamic_cast<ast::AdditiveExpressionContext*>(children[i + 1]));
    node->rht = &rht;
    node = &rht;

    i += 2;
  }

  node->rht =
    self(dynamic_cast<ast::AdditiveExpressionContext*>(children[i + 1]));

  return &ret;
}

Expr*
Ast2Asg::operator()(ast::AdditiveExpressionContext* ctx)
{
  auto& children = ctx->children;

  if (children.size() == 1)
    return self(
      dynamic_cast<ast::MultiplicativeExpressionContext*>(children[0]));

  auto& ret = make<BinaryExpr>();
  ret.lft =
    self(dynamic_cast<ast::MultiplicativeExpressionContext*>(children[0]));

  auto node = &ret;
  unsigned i = 1;
  while (true) {
    switch (dynamic_cast<antlr4::tree::TerminalNode*>(children[i])
              ->getSymbol()
              ->getType()) {
      case ast::Plus:
        node->op = node->kAdd;
        break;

      case ast::Minus:
        node->op = node->kSub;
        break;

      default:
        assert(false);
    }

    if (i >= children.size() - 2)
      break;

    auto& rht = make<BinaryExpr>();
    rht.lft = self(
      dynamic_cast<ast::MultiplicativeExpressionContext*>(children[i + 1]));
    node->rht = &rht;
    node = &rht;

    i += 2;
  }

  node->rht =
    self(dynamic_cast<ast::MultiplicativeExpressionContext*>(children[i + 1]));

  return &ret;
}

Expr*
Ast2Asg::operator()(ast::MultiplicativeExpressionContext* ctx)
{
  auto& children = ctx->children;

  if (children.size() == 1)
    return self(dynamic_cast<ast::UnaryExpressionContext*>(children[0]));

  auto& ret = make<BinaryExpr>();
  ret.lft = self(dynamic_cast<ast::UnaryExpressionContext*>(children[0]));

  auto node = &ret;
  unsigned i = 1;
  while (true) {
    switch (dynamic_cast<antlr4::tree::TerminalNode*>(children[i])
              ->getSymbol()
              ->getType()) {
      case ast::Star:
        node->op = node->kMul;
        break;

      case ast::Div:
        node->op = node->kDiv;
        break;

      case ast::Mod:
        node->op = node->kMod;
        break;

      default:
        assert(false);
    }

    if (i >= children.size() - 2)
      break;

    auto& rht = make<BinaryExpr>();
    rht.lft = self(dynamic_cast<ast::UnaryExpressionContext*>(children[i + 1]));
    node->rht = &rht;
    node = &rht;

    i += 2;
  }

  node->rht = self(dynamic_cast<ast::UnaryExpressionContext*>(children[i + 1]));

  return &ret;
}

Expr*
Ast2Asg::operator()(ast::UnaryExpressionContext* ctx)
{
  if (auto p = ctx->postfixExpression())
    return self(p);

  auto& ret = make<UnaryExpr>();

  switch (
    dynamic_cast<antlr4::tree::TerminalNode*>(ctx->unaryOperator()->children[0])
      ->getSymbol()
      ->getType()) {
    case ast::Plus:
      ret.op = ret.kPos;
      break;

    case ast::Minus:
      ret.op = ret.kNeg;
      break;

    case ast::Not:
      ret.op = ret.kNot;
      break;

    default:
      assert(false);
  }

  ret.sub = self(ctx->unaryExpression());

  return &ret;
}

Expr*
Ast2Asg::operator()(ast::PostfixExpressionContext* ctx)
{
  auto& children = ctx->children;
  auto sub = self(dynamic_cast<ast::PrimaryExpressionContext*>(children[0]));

  int i = 1;
  while (i < children.size()) {
    switch (dynamic_cast<antlr4::tree::TerminalNode*>(children[i])
              ->getSymbol()
              ->getType()) {
      case ast::LeftBracket: {
        auto& ret = make<BinaryExpr>();
        ret.op = ret.kIndex;
        ret.lft = sub;
        ret.rht = self(dynamic_cast<ast::ExpressionContext*>(children[i + 1]));
        i += 3;
        sub = &ret;
      } break;

      case ast::LeftParen: {
        auto& ret = make<CallExpr>();
        ret.head = sub;
        while (auto p = dynamic_cast<ast::ExpressionContext*>(children[++i]))
          ret.args.push_back(self(p));
        ++i;
        sub = &ret;
      } break;

      default:
        assert(false);
    }
  }

  return sub;
}

Expr*
Ast2Asg::operator()(ast::PrimaryExpressionContext* ctx)
{
  if (auto p = ctx->expression())
    return self(p);

  if (auto p = ctx->Identifier()) {
    auto name = p->getText();
    auto& ret = make<DeclRefExpr>();
    ret.decl = _localDecls->resolve(name);
    return &ret;
  }

  if (auto p = ctx->Constant()) {
    auto text = p->getText();

    auto& ret = make<IntegerLiteral>();
    ret.val = std::stoi(text);
    return &ret;
  }

  auto stringLiteral = ctx->StringLiteral();
  if (stringLiteral.size() != 0) {
    auto& ret = make<StringLiteral>();

    for (auto&& j : stringLiteral) {
      auto s = j->getText();
      std::size_t i = 1;
      while (i < s.size() - 1) {
        if (s[i] == '\\') {
          switch (s[++i]) {
            case '\n':
              break;

            case '\r':
              ++i;
              assert(s[i] == '\n');
              break;

            case '\'':
              ret.val.push_back('\'');
              break;

            case '"':
              ret.val.push_back('"');
              break;

            case '?':
              ret.val.push_back('\?');
              break;

            case '\\':
              ret.val.push_back('\\');
              break;

            case 'a':
              ret.val.push_back('\a');
              break;

            case 'b':
              ret.val.push_back('\b');
              break;

            case 'f':
              ret.val.push_back('\f');
              break;

            case 'n':
              ret.val.push_back('\n');
              break;

            case 'r':
              ret.val.push_back('\r');
              break;

            case 't':
              ret.val.push_back('\t');
              break;

            case 'v':
              ret.val.push_back('\v');
              break;

            default:
              assert(false);
          }
        }

        else {
          ret.val.push_back(s[i]);
        }

        ++i;
      }
    }

    return &ret;
  }

  assert(false);
}

/// 语句

Stmt*
Ast2Asg::operator()(ast::StatementContext* ctx)
{
  if (auto p = ctx->compoundStatement())
    return self(p);

  if (auto p = ctx->expressionStatement())
    return self(p);

  if (auto p = ctx->selectionStatement())
    return self(p);

  if (auto p = ctx->iterationStatement())
    return self(p);

  if (auto p = ctx->jumpStatement())
    return self(p);

  assert(false);
}

CompoundStmt*
Ast2Asg::operator()(ast::CompoundStatementContext* ctx)
{
  auto& ret = make<CompoundStmt>();

  if (auto p = ctx->blockItemList()) {
    LocalDecls localDecls(self);

    for (auto&& i : p->blockItem()) {
      if (auto q = i->declaration()) {
        auto& sub = make<DeclStmt>();
        sub.decls = self(q);
        ret.subs.push_back(&sub);
      }

      else if (auto q = i->statement())
        ret.subs.push_back(self(q));

      else
        assert(false);
    }
  }

  return &ret;
}

Stmt*
Ast2Asg::operator()(ast::ExpressionStatementContext* ctx)
{
  if (auto p = ctx->expression()) {
    auto& ret = make<ExprStmt>();
    ret.expr = self(p);
    return &ret;
  }

  return &make<Stmt>();
}

Stmt*
Ast2Asg::operator()(ast::SelectionStatementContext* ctx)
{
  auto& ret = make<IfStmt>();
  ret.cond = self(ctx->expression());

  auto subs = ctx->statement();
  ret.then = self(subs[0]);

  if (subs.size() > 1)
    ret.else_ = self(subs[1]);
  else
    ret.else_ = nullptr;

  return &ret;
}

Stmt*
Ast2Asg::operator()(ast::IterationStatementContext* ctx)
{
  if (ctx->Do()) {
    auto& ret = make<DoStmt>();

    {
      CurrentLoop guard(self, &ret);
      ret.body = self(ctx->statement());
    }

    ret.cond = self(ctx->expression());

    return &ret;
  }

  else {
    auto& ret = make<WhileStmt>();

    ret.cond = self(ctx->expression());

    {
      CurrentLoop guard(self, &ret);
      ret.body = self(ctx->statement());
    }

    return &ret;
  }
}

Stmt*
Ast2Asg::operator()(ast::JumpStatementContext* ctx)
{
  if (ctx->Continue()) {
    auto& ret = make<ContinueStmt>();
    assert(_currentLoop != nullptr);
    ret.loop = _currentLoop;
    return &ret;
  }

  if (ctx->Break()) {
    auto& ret = make<BreakStmt>();
    assert(_currentLoop != nullptr);
    ret.loop = _currentLoop;
    return &ret;
  }

  if (ctx->Return()) {
    auto& ret = make<ReturnStmt>();
    if (auto p = ctx->expression())
      ret.expr = self(p);
    return &ret;
  }

  assert(false);
}

/// 声明

std::vector<Decl*>
Ast2Asg::operator()(ast::DeclarationContext* ctx)
{
  std::vector<Decl*> ret;

  auto specs = self(ctx->declarationSpecifiers());

  if (auto p = ctx->initDeclaratorList()) {
    for (auto&& j : p->initDeclarator())
      ret.push_back(self(j, specs));
  }

  // 如果 initDeclaratorList 为空则这行声明语句无意义
  return ret;
}

FunctionDecl*
Ast2Asg::operator()(ast::FunctionDefinitionContext* ctx)
{
  auto& ret = make<FunctionDecl>();
  ret.type.specs = self(ctx->declarationSpecifiers());

  auto [texp, name] = self(ctx->directDeclarator());
  auto& funcType = make<FunctionType>();
  funcType.sub = texp;
  ret.type.texp = &funcType;
  ret.name = std::move(name);

  LocalDecls localDecls(self);
  if (auto p = ctx->parameterTypeList()) {
    for (auto&& i : p->parameterList()->parameterDeclaration()) {
      auto varDecl = self(i);
      ret.params.push_back(varDecl);
      funcType.params.push_back(varDecl->type);

      localDecls[varDecl->name] = varDecl; // !
    }
  }

  ret.body = self(ctx->compoundStatement());

  // 这个实现允许符号重复定义，新定义会取代旧定义
  (*_localDecls)[ret.name] = &ret;
  return &ret;
}

VarDecl*
Ast2Asg::operator()(ast::InitDeclaratorContext* ctx, Type::Specs specs)
{
  auto& ret = make<VarDecl>();

  auto [texp, name] = self(ctx->declarator());
  ret.type = { specs, texp };
  ret.name = std::move(name);

  if (auto p = ctx->initializer())
    ret.init = self(p);
  else
    ret.init = nullptr;

  // 这个实现允许符号重复定义，新定义会取代旧定义
  (*_localDecls)[ret.name] = &ret;
  return &ret;
}

VarDecl*
Ast2Asg::operator()(ast::ParameterDeclarationContext* ctx)
{
  auto& ret = make<VarDecl>();

  if (auto p = ctx->declarationSpecifiers()) {
    ret.type.specs = self(p);
    auto [texp, name] = self(ctx->declarator());
    ret.type.texp = texp;
    ret.name = std::move(name);
    ret.init = nullptr;
  }

  else if (auto p = ctx->declarationSpecifiers2()) {
    ret.type.specs = self(p);

    if (auto q = ctx->abstractDeclarator())
      ret.type.texp = self(q);
    else
      ret.type.texp = nullptr;

    ret.name = std::string();
    ret.init = nullptr;
  }

  else
    assert(false);

  return &ret;
}

Obj::Ptr<Expr, InitList>
Ast2Asg::operator()(ast::InitializerContext* ctx)
{
  if (auto p = ctx->assignmentExpression())
    return self(p);

  if (auto p = ctx->initializerList())
    return self(p);

  assert(false);
}

InitList*
Ast2Asg::operator()(ast::InitializerListContext* ctx)
{
  auto& ret = make<InitList>();

  auto& children = ctx->children;
  std::size_t i = 0;
  while (i < children.size()) {
    InitList::Elem elem;

    if (auto p = dynamic_cast<ast::DesignationContext*>(children[i])) {
      auto list = p->designatorList()->designator();

      elem.des = self(list[0]->constantExpression()->logicalOrExpression());
      for (auto j = 1; j < list.size(); ++j) {
        auto& exp = make<BinaryExpr>();
        exp.op = exp.kIndex;
        exp.lft = elem.des;
        exp.rht = self(list[j]->constantExpression()->logicalOrExpression());
        elem.des = &exp;
      }

      ++i;
    }

    if (auto p = dynamic_cast<ast::InitializerContext*>(children[i]))
      elem.val = self(p);
    else
      assert(false);

    ret.list.push_back(elem);

    i += 2; // ','
  }

  return &ret;
}

}
