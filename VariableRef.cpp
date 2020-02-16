#include "VariableRef.h"

#include "Variable.h"

VariableRef::VariableRef(const Variable* var) : var_(var) {}

int VariableRef::Priority() const {
  return var_->Priority();
}

bool VariableRef::HasFrontMinus() const {
  return var_->HasFrontMinus();
}

bool VariableRef::CheckCircular(const INodeImpl* other) const {
  return var_->CheckCircular(other);
}

std::unique_ptr<INode> VariableRef::SymCalc() const {
  return var_->SymCalc();
}

bool VariableRef::IsEqual(const INode* rh) const {
  return var_->IsEqual(rh);
}

std::unique_ptr<INode> VariableRef::Clone() const {
  return std::make_unique<VariableRef>(var_);
}

PrintSize VariableRef::Render(Canvas* canvas,
                              PrintBox print_box,
                              bool dry_run,
                              RenderBehaviour render_behaviour) const {
  if (var_->name_.empty())
    return var_->Render(canvas, print_box, dry_run, render_behaviour);
  return print_size_ = canvas->PrintAt(print_box, var_->name_, dry_run);
}

PrintSize VariableRef::LastPrintSize() const {
  if (var_->name_.empty())
    return var_->LastPrintSize();
  return print_size_;
}

Constant* VariableRef::AsConstant() {
  return const_cast<Variable*>(var_)->AsConstant();
}

const Constant* VariableRef::AsConstant() const {
  return const_cast<Variable*>(var_)->AsConstant();
}

const ErrorNode* VariableRef::AsError() const {
  return var_->AsError();
}

 Variable* VariableRef::AsVariable() {
  return const_cast<Variable*>(var_)->AsVariable();
}

const Variable* VariableRef::AsVariable() const {
  return var_->AsVariable();
}

Operation* VariableRef::AsOperation() {
  return const_cast<Variable*>(var_)->AsOperation();
}

const Operation* VariableRef::AsOperation() const {
  return var_->AsOperation();
}

bool VariableRef::SimplifyImpl(std::unique_ptr<INode>* new_node) {
  return const_cast<Variable*>(var_)->SimplifyImpl(new_node);
}
