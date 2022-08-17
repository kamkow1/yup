#include <compiler/visitor.h>

using namespace yupc;

namespace cv = compiler::visitor;

std::any cv::Visitor::visitCode_block(parser::YupParser::Code_blockContext *ctx) {

    for (auto *const statement : ctx->statement()) {
        this->visit(statement);
    }

    return nullptr;
}
