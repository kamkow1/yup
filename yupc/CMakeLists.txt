cmake_minimum_required(VERSION 3.5)
project(yupc)

set (CXX_STANDARD 20)

set (CONAN_DISABLE_CHECK_COMPILER True CACHE BOOL "" FORCE)
set (CMAKE_COMPILE_WARNING_AS_ERROR ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS 1)

set (CMAKE_CXX_STANDARD 20)
set (GCC_COVERAGE_LINK_FLAGS "--libs")

include (${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

set (THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

set (ANTLR_SRC ${THIRD_PARTY_DIR}/antlr4-runtime)

# change this to your own path
set (ANTLR_INCLUDE_PATH ${ANTLR_SRC}/runtime/src)

set (ANTLR_BUILD_PATH ${ANTLR_SRC}/dist/libantlr4-runtime.a)

include_directories (
    ${ANTLR_INCLUDE_PATH} 
    ${THIRD_PARTY_DIR}/json/include
    ${CMAKE_SOURCE_DIR}/include)

conan_basic_setup ()

find_package (Threads REQUIRED)

# LLVM
find_package (LLVM REQUIRED CONFIG)
add_definitions (${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(LLVM_LIBS support core irreader linker bitwriter bitreader)

# json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${THIRD_PARTY_DIR}/json)

set (SRCS
    src/Compiler/Logic.cpp
    src/Compiler/Configuration.cpp
    src/Compiler/FileSystem.cpp
    src/Compiler/Operator.cpp
    src/Compiler/PointerDereference.cpp
    src/Compiler/Compiler.cpp
    src/Compiler/CompilationUnit.cpp
    src/Compiler/CodeBlock.cpp
    src/Compiler/Constant.cpp
    src/Compiler/Function.cpp
    src/Compiler/Type.cpp
    src/Compiler/Visitor.cpp
    src/Compiler/Variable.cpp
    src/Compiler/Array.cpp
    src/Compiler/AddressOf.cpp
    src/Compiler/BinaryOperation.cpp
    src/Compiler/Import.cpp
    src/Lexer/YupLexer.cpp
    src/Lexer/LexerErrorListener.cpp
    src/Parser/YupParser.cpp
    src/Parser/YupParserBaseVisitor.cpp
    src/Parser/YupParserVisitor.cpp
    src/Parser/ParserErrorListener.cpp
    src/Logger.cpp
    src/main.cpp
    src/utils.cpp
)


add_executable(yupc ${SRCS})

target_link_libraries(${CMAKE_PROJECT_NAME} 
    ${ANTLR_BUILD_PATH} 
    ${CONAN_LIBS} 
    ${LLVM_LIBS} 
    Threads::Threads
    nlohmann_json::nlohmann_json)
