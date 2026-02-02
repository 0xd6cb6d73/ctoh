cmake_minimum_required(VERSION 3.28)
project(ctoh)

add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

set(Clang_DIR ${LLVM_DIR})
find_package(LLVM PATHS "${LLVM_DIR}" REQUIRED CONFIG NO_DEFAULT_PATH)
find_package(Clang PATHS "${Clang_DIR}" REQUIRED CONFIG NO_DEFAULT_PATH)

message(STATUS "Found Clang ${LLVM_PACKAGE_VERSION}")
message(STATUS "Clang includes: ${CLANG_INCLUDE_DIRS}")

target_include_directories(${PROJECT_NAME} PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE libclang)
