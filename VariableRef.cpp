#include "VariableRef.h"

#include "Variable.h"

VariableRef::VariableRef(std::weak_ptr<Variable> var)
    : var_(std::move(var)) {}

int VariableRef::Priority() const {
  return var_.lock()->Priority();
}

bool VariableRef::HasFrontMinus() const {
  return var_.lock()->HasFrontMinus();
}

bool VariableRef::CheckCircular(const INodeImpl* other) const {
  return var_.lock()->CheckCircular(other);
}

std::unique_ptr<INode> VariableRef::SymCalc() const {
  return var_.lock()->SymCalc();
}

bool VariableRef::IsEqual(const INode* rh) const {
  return var_.lock()->IsEqual(rh);
}

std::unique_ptr<INode> VariableRef::Clone() const {
  return std::make_unique<VariableRef>(var_);
}

PrintSize VariableRef::Render(Canvas* canvas,
                              PrintBox print_box,
                              bool dry_run,
                              RenderBehaviour render_behaviour) const {
  auto var = var_.lock();
  if (var->name_.empty())
    return var->Render(canvas, print_box, dry_run, render_behaviour);
  return print_size_ = canvas->PrintAt(print_box, var->name_, dry_run);
}

PrintSize VariableRef::LastPrintSize() const {
    auto var = var_.lock();
  if (var->name_.empty())
    return var->LastPrintSize();
  return print_size_;
}

Constant* VariableRef::AsConstant() {
  return var_.lock()->AsConstant();
}

const Constant* VariableRef::AsConstant() const {
  return var_.lock()->AsConstant();
}

const ErrorNode* VariableRef::AsError() const {
  return var_.lock()->AsError();
}

Variable* VariableRef::AsVariable() {
  return var_.lock()->AsVariable();
}

const Variable* VariableRef::AsVariable() const {
  return var_.lock()->AsVariable();
}

Operation* VariableRef::AsOperation() {
  return var_.lock()->AsOperation();
}

const Operation* VariableRef::AsOperation() const {
  return var_.lock()->AsOperation();
}

bool VariableRef::SimplifyImpl(std::unique_ptr<INode>* new_node) {
  return var_.lock()->SimplifyImpl(new_node);
}
