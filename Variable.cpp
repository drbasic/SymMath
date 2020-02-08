#include "Variable.h"

#include <sstream>

#include "ErrorNode.h"
#include "Exception.h"
#include "ValueHelpers.h"
#include "VariableRef.h"

Variable::Variable(std::string name) : name_(std::move(name)) {}

Variable::Variable(std::unique_ptr<INode> value) : value_(std::move(value)) {}

Variable::Variable(const Variable& var)
    : value_(std::make_unique<VariableRef>(&var)) {}

std::string Variable::PrintImpl(bool ommit_front_minus) const {
  if (name_.empty())
    return value_->PrintImpl(ommit_front_minus);
  std::stringstream ss;
  ss << name_ << "=";
  if (value_)
    ss << value_->PrintImpl(ommit_front_minus);
  else
    ss << std::string("<null>");
  return ss.str();
}

int Variable::Priority() const {
  return 100;
}

bool Variable::NeedBrackets() const {
  return false;
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

std::string Variable::PrintRef(bool ommit_front_minus) const {
  if (name_.empty())
    return value_->PrintImpl(ommit_front_minus);
  return name_;
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

bool Variable::IsUnMinus() const {
  if (auto vn = GetVisibleNode())
    return vn != this && vn->IsUnMinus();
  return false;
}

Operation* Variable::AsUnMinus() {
  if (auto vn = GetVisibleNode())
    return (vn != this) ? vn->AsUnMinus() : nullptr;
  return nullptr;
}

Constant* Variable::AsConstant() {
  if (auto vn = GetVisibleNode())
    return (vn != this) ? vn->AsConstant() : nullptr;
  return nullptr;
}

const Variable* Variable::AsVariable() const {
  return this;
}

std::vector<std::unique_ptr<INode>> Variable::TakeOperands(Op op) {
  if (auto vn = GetVisibleNode())
    return (vn != this) ? vn->TakeOperands(op) : std::vector<std::unique_ptr<INode>>{};
  return {};
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
  return std::make_unique<VariableRef>(this);
}