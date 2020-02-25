#include "ErrorNode.h"

ErrorNode::ErrorNode(std::string error) : error_(error) {}

std::unique_ptr<INode> ErrorNode::SymCalc(SymCalcSettings settings) const {
  return std::make_unique<ErrorNode>(error_);
}

std::unique_ptr<INode> ErrorNode::Clone() const {
  return std::make_unique<ErrorNode>(error_);
}

PrintSize ErrorNode::Render(Canvas* canvas,
                            PrintBox print_box,
                            bool dry_run,
                            RenderBehaviour render_behaviour) const {
  return print_size_ = canvas->PrintAt(print_box, error_,
                                       render_behaviour.GetSubSuper(), dry_run);
}

PrintSize ErrorNode::LastPrintSize() const {
  return print_size_;
}

void ErrorNode::SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) {
  token.Disarm();
}

void ErrorNode::OpenBracketsImpl(HotToken token,
                                 std::unique_ptr<INode>* new_node) {
  token.Disarm();
}

void ErrorNode::ConvertToComplexImpl(HotToken token,
                                     std::unique_ptr<INode>* new_node) {
  token.Disarm();
}

bool ErrorNode::IsEqual(const INode* rh) const {
  const ErrorNode* rh_error = rh->AsNodeImpl()->AsError();
  return rh_error && (error_ == rh_error->error_);
}
