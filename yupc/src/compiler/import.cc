#include <compiler/visitor.h>
#include <compiler/import.h>
#include <compiler/compilation_unit.h>

#include <string>
#include <vector>

using namespace yupc;

namespace cv = compiler::visitor;
namespace ci = compiler::import;
namespace com_un = compiler::compilation_unit;

void ci::append_import(std::string path) {

    com_un::comp_units.top()->module_imports.push_back(path);
}

std::any cv::Visitor::visitImport_decl(parser::YupParser::Import_declContext *ctx) {
    
    std::string path_text = ctx->V_STRING()->getText();

    path_text.erase(0, 1);
    path_text.erase(path_text.size() - 1);

    ci::append_import(path_text);

    return nullptr;
}
