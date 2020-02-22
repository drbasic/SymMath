#include "Variable.h"

#include <algorithm>
#include <cassert>
#include <sstream>

#include "ErrorNode.h"
#include "Exception.h"
#include "INodeHelper.h"
#include "Operation.h"
#include "ValueHelpers.h"
#include "VariableRef.h"

namespace {
const std::string_view kAnonimous("<anonimous>");
const std::string_view kNull("<null>");
}  // namespace

Variable::Variable(std::string name) : name_(std::move(name)) {}

Variable::Variable(std::unique_ptr<INode> value) : value_(std::move(value)) {}

Variable::Variable(std::string name, std::unique_ptr<INode> value)
    : name_(std::move(name)), value_(std::move(value)) {}

std::wstring Variable::Print() const {
  Canvas canvas;
  canvas.SetDryRun(true);
  PrintBox initial_print_box(0, 0, 1000, 1000, 0);
  RenderBehaviour render_behaviour;
  auto size = Render(&canvas, initial_print_box, true, render_behaviour);
  canvas.Resize(size);
  canvas.SetDryRun(false);
  PrintBox print_box(0, 0, size);
  auto size2 = Render(&canvas, print_box, false, render_behaviour);
  assert(size == size2);
  return canvas.ToString();
}

int Variable::Priority() const {
  if (name_.empty() && value_)
    return value_->AsNodeImpl()->Priority();
  return 100;
}

bool Variable::HasFrontMinus() const {
  if (!name_.empty())
    return false;
  if (value_)
    return value_->AsNodeImpl()->HasFrontMinus();
  return false;
}

ValueType Variable::GetValueType() const {
  if (value_)
    return value_->AsNodeImpl()->GetValueType();
  return ValueType::Scalar;
}

bool Variable::CheckCircular(const INodeImpl* other) const {
  return this == other || (Value() && Value()->CheckCircular(other));
}

bool Variable::IsEqual(const INode* rh) const {
  if (!name_.empty()) {
    const Variable* rh_var = rh->AsNodeImpl()->AsVariable();
    return rh_var && (name_ == rh_var->name_);
  }
  if (value_)
    return value_->IsEqual(rh);
  return false;
}

std::string Variable::GetName() const {
  return name_;
}

void Variable::SimplifyImpl(std::unique_ptr<INode>* new_node) {
  if (Value())
    Value()->SimplifyImpl(new_node);
}

void Variable::Simplify() {
  while (true) {
    std::unique_ptr<INode> new_node;
    SimplifyImpl(&new_node);
    if (!new_node)
      return;
    value_ = std::move(new_node);
  }
}

void Variable::OpenBrackets() {
  if (!value_)
    return;
  auto* op = INodeHelper::AsOperation(value_.get());
  if (!op)
    return;
  std::unique_ptr<INode> new_node;
  op->UnfoldChains();
  op->OpenBrackets(&new_node);
  if (new_node)
    value_ = std::move(new_node);
}

void Variable::operator=(std::unique_ptr<INode> value) {
  if (value->AsNodeImpl()->CheckCircular(this)) {
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
                           PrintBox print_box,
                           bool dry_run,
                           RenderBehaviour render_behaviour) const {
  bool print_name =
      (render_behaviour.GetVariable() != VariableBehaviour::ValueOnly);
  print_name =
      print_name &&
      ((render_behaviour.GetVariable() == VariableBehaviour::NameOnly) ||
       (!name_.empty()));
  bool print_value =
      (render_behaviour.GetVariable() != VariableBehaviour::NameOnly);
  print_value =
      print_value &&
      ((render_behaviour.GetVariable() == VariableBehaviour::ValueOnly) ||
       (value_));

  if (print_value && !print_name) {
    return print_size_ =
               RenderValue(canvas, print_box, dry_run, render_behaviour);
  }

  print_size_ =
      RenderName(canvas, print_box, dry_run, render_behaviour, print_value);
  print_box = print_box.ShrinkLeft(print_size_.width);
  if (print_value) {
    auto value_size = RenderValue(canvas, print_box, dry_run, render_behaviour);
    print_size_ = print_size_.GrowWidth(value_size, true);
  }
  return print_size_;
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

PrintSize Variable::RenderName(Canvas* canvas,
                               PrintBox print_box,
                               bool dry_run,
                               RenderBehaviour render_behaviour,
                               bool equal_sign) const {
  std::string printable_name = !name_.empty() ? name_ : std::string(kAnonimous);
  auto name_size = canvas->PrintAt(
      print_box, equal_sign ? printable_name + " = " : printable_name,
      render_behaviour.GetSubSuper(), dry_run);

  ValueType value_type =
      value_ ? value_->AsNodeImpl()->GetValueType() : ValueType::Scalar;
  if (value_type != ValueType::Vector)
    return name_size;

  print_box.base_line -= name_size.height - name_size.base_line;
  print_box.ShrinkLeft((printable_name.size() - 1)/2);
  std::wstring vector_sign(L"→");
  auto value_type_size = canvas->PrintAt(
      print_box, vector_sign, render_behaviour.GetSubSuper(), dry_run);
  return value_type_size.GrowDown(name_size, true);
}

PrintSize Variable::RenderValue(Canvas* canvas,
                                PrintBox print_box,
                                bool dry_run,
                                RenderBehaviour render_behaviour) const {
  auto rh_size =
      value_ ? Value()->Render(canvas, print_box, dry_run, render_behaviour)
             : canvas->PrintAt(print_box, kNull, render_behaviour.GetSubSuper(),
                               dry_run);
  return rh_size;
}

INodeImpl* Variable::Value() {
  return value_ ? value_->AsNodeImpl() : nullptr;
}

const INodeImpl* Variable::Value() const {
  return value_ ? value_->AsNodeImpl() : nullptr;
}

INodeImpl* Variable::GetVisibleNode() {
  if (!name_.empty())
    return this;
  if (!value_)
    return nullptr;
  auto* inner = Value();
  if (auto* inner_variable = inner->AsVariable()) {
    return inner_variable->GetVisibleNode();
  }
  return inner;
}

const INodeImpl* Variable::GetVisibleNode() const {
  if (!name_.empty())
    return this;
  if (!value_)
    return nullptr;
  auto* inner = Value();
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
