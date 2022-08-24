#pragma once

#include <llvm/IR/Instructions.h>

#include <string>
#include <map>

namespace yupc
{    
    void create_new_scope();
    void drop_scope();
}
