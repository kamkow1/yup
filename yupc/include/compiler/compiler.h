#pragma once

#include <compiler/compilation_unit.h>
#include <llvm/IR/Module.h>

#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace yupc
{
    struct CompilerOpts 
    {
        bool give_perms;
        bool verbose;
        std::vector<std::string> src_path;  
        std::string binary_name;
    };

    extern CompilerOpts compiler_opts;

    void build_bitcode(fs::path bin_file);

    void process_source_file(std::string path);

    void process_path(std::string path);

    std::string create_module_name(std::string base_path);
};
