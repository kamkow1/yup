cmake_minimum_required(VERSION 3.5)
project(yupc)

set (CMAKE_CXX_STANDARD 20)
set (GCC_COVERAGE_LINK_FLAGS "--libs")

include (${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

# change this to your own path
set (ANTLR_INCLUDE_PATH
        /archive/ARCHIVE/antlr4-cpp-runtime/runtime/src
        /archive/ARCHIVE/antlr4-cpp-runtime/runtime/src/misc
        /archive/ARCHIVE/antlr4-cpp-runtime/runtime/src/atn
        /archive/ARCHIVE/antlr4-cpp-runtime/runtime/src/dfa
        /archive/ARCHIVE/antlr4-cpp-runtime/runtime/src/tree
        /archive/ARCHIVE/antlr4-cpp-runtime/runtime/src/support
        )

set (LLVM_INCLUDE_PATH /archive/ARCHIVE/llvm-project/llvm/build/include)

set (ANTLR_BUILD_PATH /archive/ARCHIVE/antlr4-cpp-runtime/dist/libantlr4-runtime.so)

include_directories (${ANTLR_INCLUDE_PATH} ${LLVM_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/include)

conan_basic_setup ()

find_package (Threads REQUIRED)
find_package (LLVM REQUIRED CONFIG)

add_definitions (${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core)

set (SRCS
    src/ast_visitor/code_block.cc
    src/ast_visitor/constant.cc
    src/ast_visitor/data.cc
    src/ast_visitor/function.cc
    src/ast_visitor/type.cc
    src/ast_visitor/visitor.cc
    src/ast_visitor/variable.cc
    src/lexer/YupLexer.cc
    src/messaging/errors.cc
    src/messaging/information.cc
    src/parser/YupParser.cc
    src/parser/YupParserBaseVisitor.cpp
    src/parser/YupParserVisitor.cpp
    src/main.cc
    src/parser_error_listener.cc
    src/util.cc
)


add_executable(${CMAKE_PROJECT_NAME} ${SRCS})

target_link_libraries(${CMAKE_PROJECT_NAME} ${ANTLR_BUILD_PATH} ${CONAN_LIBS} ${llvm_libs} Threads::Threads)
