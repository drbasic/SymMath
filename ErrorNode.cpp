#include "ErrorNode.h"

ErrorNode::ErrorNode(std::string error) : error_(error) {}

std::unique_ptr<INode> ErrorNode::SymCalc() const {
  return std::make_unique<ErrorNode>(error_);
}

std::unique_ptr<INode> ErrorNode::Clone() const {
  return std::make_unique<ErrorNode>(error_);
}

PrintSize ErrorNode::Render(Canvas* canvas,
                            PrintBox print_box,
                            bool dry_run,
                            RenderBehaviour render_behaviour) const {
  return canvas->PrintAt(print_box, error_, dry_run);
}

PrintSize ErrorNode::LastPrintSize() const {
  return print_size_;
}

int ErrorNode::Priority() const {
  return 0;
}

bool ErrorNode::HasFrontMinus() const {
  return false;
}

bool ErrorNode::CheckCircular(const INode* other) const {
  return false;
}

bool ErrorNode::IsEqual(const INode* rh) const {
  const ErrorNode* rh_error = rh->AsError();
  return rh_error && (error_ == rh_error->error_);
}
