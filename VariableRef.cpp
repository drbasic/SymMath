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

INode* VariableRef::GetVisibleNode() {
  return var_->GetVisibleNodeImpl();
}

const INode* VariableRef::GetVisibleNode() const {
  return var_->GetVisibleNode();
}

bool VariableRef::IsUnMinus() const
{
  return var_->IsUnMinus();
}

bool VariableRef::SimplifyImpl(std::unique_ptr<INode>* new_node) {
  return const_cast<Variable*>(var_)->SimplifyImpl(new_node);
}

std::string VariableRef::GetName() const {
  return var_->GetName();
}