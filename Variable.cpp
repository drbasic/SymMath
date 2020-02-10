#include "Variable.h"

#include <cassert>
#include <sstream>

#include "ErrorNode.h"
#include "Exception.h"
#include "ValueHelpers.h"
#include "VariableRef.h"

namespace {
const std::string_view kAnonimous("<anonimous>");
const std::string_view kNull("<null>");
}

Variable::Variable(std::string name) : name_(std::move(name)) {}

Variable::Variable(std::unique_ptr<INode> value) : value_(std::move(value)) {}

Variable::Variable(const Variable& var)
    : value_(std::make_unique<VariableRef>(&var)) {}

std::string Variable::Print() const {
  return "!!!";
}

int Variable::Priority() const {
  return 100;
}

bool Variable::HasFrontMinus() const {
  if (!name_.empty())
    return false;
  if (value_)
    return value_->HasFrontMinus();
  return false;
}

bool Variable::CheckCircular(const INode* other) const {
  return this == other || (value_ && value_->CheckCircular(other));
}

bool Variable::IsEqual(const INode* rh) const {
  if (!name_.empty()) {
    const Variable* rh_var = rh->AsVariable();
    return rh_var && (name_ == rh_var->name_);
  }
  if (value_)
    return value_->IsEqual(rh);
  return false;
}

std::string Variable::GetName() const {
  return name_;
}

bool Variable::SimplifyImpl(std::unique_ptr<INode>* new_node) {
  if (!value_)
    return false;
  return value_->SimplifyImpl(new_node);
}

bool Variable::Simplify() {
  bool simplified = false;
  std::unique_ptr<INode> new_node;
  while (SimplifyImpl(&new_node)) {
    simplified = true;
    if (new_node)
      value_ = std::move(new_node);
  }
  return simplified;
}

void Variable::operator=(std::unique_ptr<INode> value) {
  if (value->CheckCircular(this)) {
    value_ = std::make_unique<ErrorNode>(
        "Circular deps on [" + (name_.empty() ? "<unonimous>" : name_) + "]");
  } else {
    value_ = std::move(value);
  }
}

void Variable::operator=(const Variable& var) {
  if (this == &var)
    return;
  if (var.CheckCircular(this)) {
    value_ = std::make_unique<ErrorNode>(
        "Circular deps on [" + (name_.empty() ? "<unonimous>" : name_) + "]");
  } else {
    value_ = std::make_unique<VariableRef>(&var);
  }
}

void Variable::operator=(double val) {
  value_ = Const(val);
}

std::unique_ptr<INode> Variable::SymCalc() const {
  if (!value_)
    return std::make_unique<VariableRef>(this);
  return value_->SymCalc();
}

std::unique_ptr<INode> Variable::Clone() const {
  assert(false);
  return std::unique_ptr<INode>();
}

PrintSize Variable::Render(Canvas* canvas,
                           const Position& pos,
                           bool dry_run,
                           bool ommit_front_minus) const {
  if (name_.empty()) {
    if (value_)
      return value_->Render(canvas, pos, dry_run, ommit_front_minus);
  }

  std::stringstream ss;
  ss << (name_.empty() ? name_ : kAnonimous);
  ss << "=";
  auto str = ss.str();
  PrintSize lh_size{str.size(), 1};
  PrintSize rh_size;
  if (value_) {
    rh_size = value_->PrintImpl(canvas, {pos.x + lh_size.width}, dry_run,
                                ommit_front_minus);
  } else {
    rh_size = {std::string(kNull).size(), 1};
  }
  if (!dry_run) {
    canvas->PrintAt({pos.x, pos.y + rh_size.height / 2}, str);
  }
  return {lh_size.width + rh_size.width,
          std::max(lh_size.width, rh_size.width)};
}

Constant* Variable::AsConstant() {
  if (auto vn = GetVisibleNode())
    return (vn != this) ? vn->AsConstant() : nullptr;
  return nullptr;
}

const Constant* Variable::AsConstant() const {
  if (auto vn = GetVisibleNode())
    return (vn != this) ? vn->AsConstant() : nullptr;
  return nullptr;
}

const ErrorNode* Variable::AsError() const {
  if (auto vn = GetVisibleNode())
    return (vn != this) ? vn->AsError() : nullptr;
  return nullptr;
}

const Variable* Variable::AsVariable() const {
  return this;
}

Operation* Variable::AsOperation() {
  if (auto vn = GetVisibleNode())
    return (vn != this) ? vn->AsOperation() : nullptr;
  return nullptr;
}

const Operation* Variable::AsOperation() const {
  if (auto vn = GetVisibleNode())
    return (vn != this) ? vn->AsOperation() : nullptr;
  return nullptr;
}

INode* Variable::GetVisibleNode() const {
  if (!name_.empty())
    return const_cast<INode*>(static_cast<const INode*>(this));
  INode* inner = const_cast<INode*>(value_.get());
  if (!inner)
    return nullptr;
  if (auto* inner_variable = inner->AsVariable()) {
    return inner_variable->GetVisibleNode();
  }
  return inner;
}

Variable::operator std::unique_ptr<INode>() const {
  if (!name_.empty())
    return std::make_unique<VariableRef>(this);
  if (!value_)
    return std::make_unique<ErrorNode>("bind to empty unnamed var");
  return value_->Clone();
}
