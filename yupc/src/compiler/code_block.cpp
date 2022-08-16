#include <compiler/visitor.h>
#include <compiler/auto_deleter.h>

using namespace yupc;

namespace cv = compiler::visitor;
namespace auto_del = compiler::auto_deleter;

std::any cv::Visitor::visitCode_block(parser::YupParser::Code_blockContext *ctx) {

    for (auto *const statement : ctx->statement()) {
        this->visit(statement);
    }

    for (auto i = 0; i < auto_del::marked_vars.size(); i++) {

        auto_del::free_call_codegen(i);
    }

    return nullptr;
}
