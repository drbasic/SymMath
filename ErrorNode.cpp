#include "ErrorNode.h"

ErrorNode::ErrorNode(std::string error) : error_(error) {}

std::unique_ptr<INode> ErrorNode::SymCalc() const {
  return std::make_unique<ErrorNode>(error_);
}

std::string ErrorNode::PrintImpl(bool ommit_front_minus) const {
  return error_;
}

int ErrorNode::Priority() const {
  return 0;
}

bool ErrorNode::NeedBrackets() const
{
  return false;
}

bool ErrorNode::HasFrontMinus() const
{
  return false;
}

bool ErrorNode::CheckCircular(const INode* other) const
{
  return false;
}

bool ErrorNode::IsUnMinus() const {
  return false;
}
