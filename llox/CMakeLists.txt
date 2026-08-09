cmake_minimum_required(VERSION 3.20)

project(llox
        VERSION 1.0.0
        LANGUAGES CXX)

# User Modern C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependency: flex
find_package(FLEX REQUIRED)
FLEX_TARGET(LoxScanner src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


# Target: llox
set(SOURCES
    src/main.cc
    src/parser.cc
    ${FLEX_LoxScanner_OUTPUTS}
)
add_executable(llox ${SOURCES})
target_include_directories(llox PRIVATE include/)

# Add compiler warnings to ensure high code quality
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(llox PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(MSVC)
    target_compile_options(llox PRIVATE /W4 /WX)
endif()

# ----------
# Testing
enable_testing()

set(TEST_SOURCES
    src/parser.cc
    tests/test_expr.cc
    ${FLEX_LoxScanner_OUTPUTS}
)
add_executable(llox_test ${TEST_SOURCES})
target_include_directories(llox_test PRIVATE include/)
