cmake_minimum_required(VERSION 4.2)
project(ctoh VERSION 0.1 DESCRIPTION "Clang-based header generation for BOF to BOF-PE conversion" LANGUAGES CXX)

add_executable(ctoh src/main.cpp)

add_library(ctoh.lib STATIC)
add_library(ctoh::lib ALIAS ctoh.lib)
target_sources(ctoh.lib
  PUBLIC
    FILE_SET CXX_MODULES FILES
      src/enum.cppm src/ctoh.lib.cppm src/shared.cppm src/functions.cppm src/typedefs.cppm src/structs.cppm src/unions.cppm
)
target_compile_features(ctoh.lib PUBLIC cxx_std_23)
target_link_libraries(ctoh.lib PRIVATE libclang)
target_include_directories(ctoh.lib PRIVATE ${CLANG_INCLUDE_DIRS})

set(Clang_DIR ${LLVM_DIR})
find_package(LLVM PATHS "${LLVM_DIR}" REQUIRED CONFIG NO_DEFAULT_PATH)
find_package(Clang PATHS "${Clang_DIR}" REQUIRED CONFIG NO_DEFAULT_PATH)

message(STATUS "Found Clang ${LLVM_PACKAGE_VERSION}")
message(STATUS "Clang includes: ${CLANG_INCLUDE_DIRS}")

target_include_directories(ctoh PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(ctoh PRIVATE ctoh::lib)
