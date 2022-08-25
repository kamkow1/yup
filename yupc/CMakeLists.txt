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
    src/compiler/Configuration.cpp
    src/compiler/FileSystem.cpp
    src/compiler/Operator.cpp
    src/compiler/PointerDereference.cpp
    src/compiler/Compiler.cpp
    src/compiler/CompilationUnit.cpp
    src/compiler/CodeBlock.cpp
    src/compiler/Constant.cpp
    src/compiler/Function.cpp
    src/compiler/Type.cpp
    src/compiler/Visitor.cpp
    src/compiler/Variable.cpp
    src/compiler/Array.cpp
    src/compiler/AddressOf.cpp
    src/compiler/BinaryOperation.cpp
    src/compiler/Import.cpp
    src/Logger.cpp
    src/lexer/YupLexer.cpp
    src/lexer/LexerErrorListener.cpp
    src/parser/YupParser.cpp
    src/parser/YupParserBaseVisitor.cpp
    src/parser/YupParserVisitor.cpp
    src/parser/ParserErrorListener.cpp
    src/main.cpp
    src/utils.cpp
)


add_executable(yupc ${SRCS})

target_link_libraries(${CMAKE_PROJECT_NAME} ${ANTLR_BUILD_PATH} ${CONAN_LIBS} ${llvm_libs} Threads::Threads)
