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
    };

    extern std::string build_dir;
    extern CompilerOpts compiler_opts;

    void dump_module(llvm::Module *module, std::string module_name);

    void build_binary(fs::path bin_file, fs::path obj_dir);

    void output_obj(std::string s_path);

    void process_source_file(std::string path);

    std::string init_build_dir(std::string dir_base);
};
