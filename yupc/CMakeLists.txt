cmake_minimum_required(VERSION 3.5)
project(yupc)

set (CONAN_DISABLE_CHECK_COMPILER True CACHE BOOL "" FORCE)

set (CMAKE_CXX_STANDARD 20)
set (GCC_COVERAGE_LINK_FLAGS "--libs")

include (${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

set (THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

set (ANTLR_SRC ${THIRD_PARTY_DIR}/antlr4-runtime)

set (LLVM_SRC ${THIRD_PARTY_DIR}/llvm)

# change this to your own path
set (ANTLR_INCLUDE_PATH
        ${ANTLR_SRC}/runtime/src
        ${ANTLR_SRC}/runtime/src/misc
        ${ANTLR_SRC}/runtime/src/atn
        ${ANTLR_SRC}/runtime/src/dfa
        ${ANTLR_SRC}/runtime/src/tree
        ${ANTLR_SRC}/runtime/src/support
        )

set (LLVM_INCLUDE_PATH ${LLVM_SRC}/build/include)

set (ANTLR_BUILD_PATH ${ANTLR_SRC}/dist/libantlr4-runtime.a)

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
    src/compiler/math.cc
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
