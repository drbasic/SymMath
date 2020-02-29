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
const std::wstring_view kAnonimous(L"<anonimous>");
const std::string_view kNull("<null>");
const std::string_view kArrow(" -> ");
}  // namespace

Variable::Variable(std::wstring name) : name_(std::move(name)) {}

Variable::Variable(std::unique_ptr<INode> value) : value_(std::move(value)) {}

Variable::Variable(std::wstring name, std::unique_ptr<INode> value)
    : name_(std::move(name)), value_(std::move(value)) {}

Variable::~Variable() {}

std::wstring Variable::Print(bool with_calc, uint32_t base_line) const {
  RenderBehaviour render_behaviour;

  Canvas canvas;
  canvas.SetDryRun(true);
  PrintBox initial_print_box(0, 0, 10000, 10000, 0);

  auto value_size = Render(&canvas, initial_print_box, true, render_behaviour);
  assert(value_size == LastPrintSize());
  auto total_size(value_size);

  Variable calculated_value = Const(0);
  if (with_calc) {
    calculated_value = SymCalc(SymCalcSettings::KeepNamedConstants);
    calculated_value.OpenBrackets();
    calculated_value.Simplify();

    auto calculated_value_size = calculated_value.Render(
        &canvas, initial_print_box, true, render_behaviour);
    assert(calculated_value_size == calculated_value.LastPrintSize());
    auto arrow_size = canvas.PrintAt(initial_print_box, kArrow,
                                     render_behaviour.GetSubSuper(), true);
    total_size = value_size.GrowWidth(arrow_size, true)
                     .GrowWidth(calculated_value_size, true);
  }
  if (base_line > total_size.base_line) {
    total_size.height += base_line - total_size.base_line;
    total_size.base_line = base_line;
  }
  canvas.Resize(total_size);
  canvas.SetDryRun(false);
  PrintBox print_box(total_size, 0, 0);
  auto value_size2 = Render(&canvas, print_box, false, render_behaviour);
  auto total_size2(value_size2);
  assert(value_size == value_size2);
  assert(value_size == LastPrintSize());

  if (with_calc) {
    print_box = print_box.ShrinkLeft(value_size.width);
    auto arrow_size = canvas.PrintAt(print_box, kArrow,
                                     render_behaviour.GetSubSuper(), false);
    print_box = print_box.ShrinkLeft(arrow_size.width);
    auto calculated_value_size =
        calculated_value.Render(&canvas, print_box, false, render_behaviour);
    assert(calculated_value_size == calculated_value.LastPrintSize());
    total_size2 = value_size.GrowWidth(arrow_size, true)
                      .GrowWidth(calculated_value_size, true);
  }
  if (base_line == 0)
    assert(total_size == total_size2);
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

CompareResult Variable::Compare(const INode* rh) const {
  auto result = CompareType(rh);
  if (result != CompareResult::Equal)
    return result;
  const Variable* rh_variable = rh->AsNodeImpl()->AsVariable();
  assert(rh_variable);
  result = CompareTrivial(GetName(), rh_variable->GetName());
  return result;
}

std::wstring Variable::GetName() const {
  return name_;
}

void Variable::SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) {
  if (Value())
    Value()->SimplifyImpl({&token}, new_node);
  else
    token.Disarm();
}

void Variable::OpenBracketsImpl(HotToken token,
                                std::unique_ptr<INode>* new_node) {
  if (Value())
    Value()->OpenBracketsImpl({&token}, new_node);
  else
    token.Disarm();
}

void Variable::ConvertToComplexImpl(HotToken token,
                                    std::unique_ptr<INode>* new_node) {
  if (Value())
    Value()->ConvertToComplexImpl({&token}, new_node);
  else
    token.Disarm();
}

void Variable::Simplify() {
  while (true) {
    HotToken token;
    std::unique_ptr<INode> new_node;
    SimplifyImpl({&token}, &new_node);
    if (new_node)
      value_ = std::move(new_node);
    else if (token.GetChangesCount() == 0)
      break;
  }
}

void Variable::OpenBrackets() {
  if (!value_)
    return;
  HotToken token;
  std::unique_ptr<INode> temp_node;
  value_->AsNodeImpl()->OpenBracketsImpl({&token}, &temp_node);
  if (temp_node)
    value_ = std::move(temp_node);
}

void Variable::ConvertToComplex() {
  if (!value_)
    return;
  HotToken token;
  std::unique_ptr<INode> temp_node;
  value_->AsNodeImpl()->ConvertToComplexImpl({&token}, &temp_node);
  if (temp_node)
    value_ = std::move(temp_node);
}

void Variable::operator=(std::unique_ptr<INode> value) {
  if (value->AsNodeImpl()->CheckCircular(this)) {
    value_ = std::make_unique<ErrorNode>(
        L"Circular deps on [" + (name_.empty() ? L"<unonimous>" : name_) +
        L"]");
  } else {
    value_ = std::move(value);
  }
}

void Variable::operator=(const Variable& var) {
  if (this == &var)
    return;
  if (var.CheckCircular(this)) {
    value_ = std::make_unique<ErrorNode>(
        L"Circular deps on [" + (name_.empty() ? L"<unonimous>" : name_) +
        L"]");
  } else {
    value_ = std::make_unique<VariableRef>(&var);
  }
}

void Variable::operator=(double val) {
  value_ = Const(val);
}

std::unique_ptr<INode> Variable::SymCalc(SymCalcSettings settings) const {
  if (!value_)
    return std::make_unique<VariableRef>(this);
  return value_->SymCalc(settings);
}

std::unique_ptr<INode> Variable::Clone() const {
  assert(false);
  return std::unique_ptr<INode>();
}

NodeType Variable::GetNodeType() const {
  if (auto vn = GetVisibleNode())
    return (vn != this) ? vn->GetNodeType() : NodeType::Variable;
  return NodeType::Variable;
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
  std::wstring printable_name =
      !name_.empty() ? name_ : std::wstring(kAnonimous);
  auto name_size = canvas->PrintAt(
      print_box, equal_sign ? printable_name + L" = " : printable_name,
      render_behaviour.GetSubSuper(), dry_run);

  ValueType value_type =
      value_ ? value_->AsNodeImpl()->GetValueType() : ValueType::Scalar;
  if (value_type != ValueType::Vector)
    return name_size;

  print_box.base_line -= name_size.height - name_size.base_line;
  print_box = print_box.ShrinkLeft(
      static_cast<uint32_t>(printable_name.size() - 1) / 2);
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
    return std::make_unique<ErrorNode>(L"bind to empty unnamed var");
  return value_->Clone();
}
