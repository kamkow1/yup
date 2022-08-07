#pragma once

#include "compiler/compilation_unit.h"
#include <llvm/IR/Module.h>

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace yupc::compiler
{
    struct CompilerOpts {
        bool emitIR;
        bool givePerms;
        bool verbose;

        std::string srcPath;
    };

    extern std::string build_dir;
    extern CompilerOpts compiler_opts;

    void dump_module(llvm::Module *module, std::string module_name);

    void build_binary(fs::path bin_file, fs::path obj_dir);

    void output_obj(std::string s_path);

    void process_source_file(std::string path);

    std::string init_build_dir(std::string dir_base);
};
