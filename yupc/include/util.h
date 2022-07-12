#pragma once
#include <map>
#include <filesystem>
#include <messaging/errors.h>
#include <iostream>
#include <llvm/IR/Type.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <visitor.h>

constexpr unsigned int strToInt(const char* str, int h = 0);

llvm::Type* matchBasicType(std::string typeName);

std::string fileToString(const std::string& path);

std::string dirnameOf(const std::string& fname);

std::string getIRFileName(std::string path);

std::string getReadableTypeName(std::string typeName);