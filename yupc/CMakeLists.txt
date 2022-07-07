cmake_minimum_required (VERSION 3.16.3)

project (yupc)

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

set (ANTLR_BUILD_PATH /archive/ARCHIVE/antlr4-cpp-runtime/dist/libantlr4-runtime.so)

include_directories (${ANTLR_INCLUDE_PATH})

conan_basic_setup ()

find_package (Threads REQUIRED)

add_executable (yupc
    src/main.cc
    src/lexer/YupLexer.cc
    src/parser/YupParser.cc
    src/ast_visitor/visitor.cc
    )

target_link_libraries (yupc ${ANTLR_BUILD_PATH} ${CONAN_LIBS} Threads::Threads)