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
    ${CMAKE_SOURCE_DIR}/include)

conan_basic_setup ()

find_package (Threads REQUIRED)
find_package (LLVM REQUIRED CONFIG)

add_definitions (${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader linker bitwriter bitreader)

set (SRCS
    src/compiler/config.cpp
    src/compiler/file_sys.cpp
    src/compiler/operators.cpp
    src/compiler/ptr_dereference.cpp
    src/compiler/compiler.cpp
    src/compiler/compilation_unit.cpp
    src/compiler/code_block.cpp
    src/compiler/constant.cpp
    src/compiler/function.cpp
    src/compiler/type.cpp
    src/compiler/visitor.cpp
    src/compiler/variable.cpp
    src/compiler/array.cpp
    src/compiler/addr_of.cpp
    src/compiler/math.cpp
    src/compiler/import.cpp
    src/msg/errors.cpp
    src/msg/info.cpp
    src/lexer/YupLexer.cpp
    src/lexer/lexer_error_listener.cpp
    src/parser/YupParser.cpp
    src/parser/YupParserBaseVisitor.cpp
    src/parser/YupParserVisitor.cpp
    src/parser/parser_error_listener.cpp
    src/main.cpp
    src/utils.cpp
)


add_executable(yupc ${SRCS})

target_link_libraries(${CMAKE_PROJECT_NAME} ${ANTLR_BUILD_PATH} ${CONAN_LIBS} ${llvm_libs} Threads::Threads)
