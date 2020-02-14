#include "Variable.h"

#include <algorithm>
#include <cassert>
#include <sstream>

#include "ErrorNode.h"
#include "Exception.h"
#include "ValueHelpers.h"
#include "VariableRef.h"

namespace {
const std::string_view kAnonimous("<anonimous>");
const std::string_view kNull("<null>");
}  // namespace

Variable::Variable(std::string name) : name_(std::move(name)) {}

Variable::Variable(std::unique_ptr<INode> value) : value_(std::move(value)) {}

Variable::Variable(const Variable& var)
    : value_(std::make_unique<VariableRef>(&var)) {}

std::wstring Variable::Print() const {
  Canvas canvas;
  canvas.SetDryRun(true);
  auto size = Render(&canvas, {}, true, MinusBehavior::Relax);
  canvas.Resize({size.width + 3, size.height + 2});
  canvas.SetDryRun(false);
  auto size2 = Render(&canvas, {3, 2}, false, MinusBehavior::Relax);
  assert(size == size2);
  return canvas.ToString();
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
                           MinusBehavior minus_behavior) const {
  if (name_.empty()) {
    if (value_) {
      return print_size_ = value_->Render(canvas, pos, dry_run, minus_behavior);
    }
  }

  std::string var_printable_name =
      (!name_.empty() ? name_ : std::string(kAnonimous)) + "=";

  size_t base_line = 0;
  if (!dry_run && value_)
    base_line = value_->LastPrintSize().base_line;
  auto lh_size =
      canvas->PrintAt({pos.x, pos.y + base_line}, var_printable_name, dry_run);
  PrintSize rh_size;
  if (value_) {
    rh_size = value_->Render(canvas, {pos.x + lh_size.width, pos.y}, dry_run,
                             minus_behavior);
  } else {
    rh_size = canvas->PrintAt({pos.x + lh_size.width, pos.y}, kNull, dry_run);
  }
  lh_size.GrowRight(rh_size);
  return print_size_ = lh_size;
}

PrintSize Variable::LastPrintSize() const {
  return print_size_;
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
