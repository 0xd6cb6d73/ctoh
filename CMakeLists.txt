cmake_minimum_required(VERSION 4.2)
project(ctoh VERSION 0.1 DESCRIPTION "Clang-based header generation for BOF to BOF-PE conversion" LANGUAGES CXX)

add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

set(Clang_DIR ${LLVM_DIR})
find_package(LLVM PATHS "${LLVM_DIR}" REQUIRED CONFIG NO_DEFAULT_PATH)
find_package(Clang PATHS "${Clang_DIR}" REQUIRED CONFIG NO_DEFAULT_PATH)

message(STATUS "Found Clang ${LLVM_PACKAGE_VERSION}")
message(STATUS "Clang includes: ${CLANG_INCLUDE_DIRS}")

target_include_directories(${PROJECT_NAME} PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE libclang)
