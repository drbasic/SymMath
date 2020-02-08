#include "INode.h"

std::string INode::Print() const
{
  return PrintImpl(false);
}
