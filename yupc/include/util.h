#pragma once
#include "map"
#include "filesystem"
#include "messaging/errors.h"
#include "iostream"
#include "llvm/IR/Type.h"
#include "string"
#include "fstream"
#include "filesystem"
#include "visitor.h"

std::string fileToString(const std::string& path);

std::string dirnameOf(const std::string& fname);

std::string getIRFileName(std::string path);
