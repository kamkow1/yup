cmake_minimum_required(VERSION 3.16.3)

project(yupc)

set(CMAKE_CXX_STANDARD 20)
set(GCC_COVERAGE_LINK_FLAGS "-wl" "--no-as-needed" "--libs")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

# change this to your own path
set(ANTLR_INCLUDE_PATH /archive/ARCHIVE/antlr4/runtime/Cpp/runtime/src)
set(ANTLR_BUILD_PATH /archive/ARCHIVE/antlr4/runtime/Cpp/dist/libantlr4-runtime.so)

include_directories(${ANTLR_INCLUDE_PATH})

conan_basic_setup()

add_executable(yupc 
    src/main.cc
    src/lexer/YupLexer.cc
    src/parser/YupParser.cc
    src/ast_visitor/visitor.cc
)

target_link_libraries(yupc ${ANTLR_BUILD_PATH} ${CONAN_LIBS})