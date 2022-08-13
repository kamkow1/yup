#pragma once

#include "compiler/compilation_unit.h"
#include <llvm/IR/Module.h>

#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace yupc::compiler
{
    struct CompilerOpts {
        bool give_perms;
        bool verbose;
        std::vector<std::string> src_path;  
        std::string binary_name;
        std::vector<std::string> external_imports;
    };

    extern std::string build_dir;
    extern CompilerOpts compiler_opts;

    void dump_module(llvm::Module *module, std::string module_name);

    void build_bitcode(fs::path bin_file, fs::path ll_dir);

    void process_source_file(std::string path);

    std::string init_build_dir(std::string dir_base);
};
