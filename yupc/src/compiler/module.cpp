#include <compiler/compilation_unit.h>
#include <compiler/module.h>
#include <compiler/visitor.h>

#include <vector>
#include <string>

void yupc::set_module_id(std::string new_id) 
{
    yupc::comp_units.back()->module_id = new_id;
}

std::any yupc::Visitor::visitModule_decl(yupc::YupParser::Module_declContext *ctx) 
{
    std::string module_name = ctx->IDENTIFIER()->getText();
    
    yupc::comp_units.back()->module_id = module_name;
    return nullptr;
}
