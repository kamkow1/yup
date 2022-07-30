cmake_minimum_required(VERSION 3.5)
project(yupc)

set (CONAN_DISABLE_CHECK_COMPILER True CACHE BOOL "" FORCE)

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
    src/compiler/code_block.cc
    src/compiler/constant.cc
    src/compiler/function.cc
    src/compiler/type.cc
    src/compiler/visitor.cc
    src/compiler/variable.cc
    src/compiler/array.cc
    src/compiler/addr_of.cc
    src/lexer/YupLexer.cc
    src/messaging/errors.cc
    src/messaging/information.cc
    src/parser/YupParser.cc
    src/parser/YupParserBaseVisitor.cc
    src/parser/YupParserVisitor.cc
    src/main.cc
    src/parser_error_listener.cc
    src/util.cc
)


add_executable(yupc ${SRCS})

target_link_libraries(${CMAKE_PROJECT_NAME} ${ANTLR_BUILD_PATH} ${CONAN_LIBS} ${llvm_libs} Threads::Threads)
