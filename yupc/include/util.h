#pragma once
#include "map"
#include "filesystem"
#include "messaging/errors.h"
#include "iostream"
#include "llvm/IR/Type.h"
#include "string"
#include "fstream"
#include "filesystem"
#include "compiler/visitor.h"

std::string file_to_str(const std::string& path);

std::string get_dir_name(const std::string& fname);

std::string get_ir_fname(std::string path);
