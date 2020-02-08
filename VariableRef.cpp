#include "VariableRef.h"

#include "Variable.h"

VariableRef::VariableRef(const Variable* var) : var_(var) {}

std::string VariableRef::PrintImpl(bool ommit_front_minus) const {
  return var_->PrintRef(ommit_front_minus);
}

int VariableRef::Priority() const
{
  return var_->Priority();
}

bool VariableRef::NeedBrackets() const {
  return var_->NeedBrackets();
}

bool VariableRef::HasFrontMinus() const {
  return var_->HasFrontMinus();
}

bool VariableRef::CheckCircular(const INode* other) const {
  return var_->CheckCircular(other);
}

std::unique_ptr<INode> VariableRef::SymCalc() const {
  return var_->SymCalc();
}

bool VariableRef::IsUnMinus() const
{
  return var_->IsUnMinus();
}

Operation* VariableRef::AsUnMinus()
{
  return const_cast<Variable*>(var_)->AsUnMinus();
}

Constant* VariableRef::AsConstant()
{
  return const_cast<Variable*>(var_)->AsConstant();
}

const Variable* VariableRef::AsVariable() const
{
  return var_->AsVariable();
}

std::vector<std::unique_ptr<INode>> VariableRef::TakeOperands(Op op)
{
  return const_cast<Variable*>(var_)->TakeOperands(op);
}

bool VariableRef::SimplifyImpl(std::unique_ptr<INode>* new_node) {
  return const_cast<Variable*>(var_)->SimplifyImpl(new_node);
}

std::string VariableRef::GetName() const {
  return var_->GetName();
}