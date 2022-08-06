#include <compiler/visitor.h>

using namespace yupc;

namespace cv = compiler::visitor;

std::any cv::Visitor::visitCode_block(parser::YupParser::Code_blockContext *ctx) {
    for (parser::YupParser::StatementContext *const statement : ctx->statement()) {
        this->visit(statement);
    }

    return nullptr;
}
