cmake_minimum_required (VERSION 3.25)
project (simpletest VERSION 1.0 LANGUAGES CXX)
configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
                "${PROJECT_SOURCE_DIR}/src/version.h")

execute_process(COMMAND which clang++
                OUTPUT_VARIABLE clang_path)
string(STRIP ${clang_path} clang_path)
message(${clang_path})
message(${PROJECT_NAME})
set(CMAKE_CXX_COMPILER ${clang_path})
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/build)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g")

add_compile_options(-Wall -Wextra)
add_link_options(-stdlib=libc++)

add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/gflags)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/glog)

set(SRC
${PROJECT_SOURCE_DIR}/src/main.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
add_executable($(PROJECT_NAME) ${SRC})

target_link_libraries ($(PROJECT_NAME)
PUBLIC glog::glog
PUBLIC gflags::gflags)
