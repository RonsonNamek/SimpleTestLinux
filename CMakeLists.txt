cmake_minimum_required (VERSION 3.26)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "
    FATAL: In-source builds are not allowed.
           You should create a separate directory for build files.
           ")
endif()

# 临时设置编译器为 clang++
#execute_process(COMMAND which clang++
#                OUTPUT_VARIABLE CLANG_PATH)
#string(STRIP "${CLANG_PATH}" CLANG_PATH)
set(CMAKE_CXX_COMPILER clang++)

# 默认编译类型为 Release
if (NOT CMAKE_BUILD_TYPE)
    if ("${CMAKE_BINARY_DIR}" MATCHES ".*(D|d)ebug$")
        message(STATUS "No build type selected, default to *** Debug ***")
        set(CMAKE_BUILD_TYPE "Debug")
    else()
        message(STATUS "No build type selected, default to *** Release ***")
        set(CMAKE_BUILD_TYPE "Release")
    endif()
endif()

# 设置版本号、项目名称
file(READ "version.txt" VERSION_FILE)
string(STRIP "${VERSION_FILE}" VERSION_FILE)
project(simpletest VERSION ${VERSION_FILE}
    DESCRIPTION "Very nice test project"
    LANGUAGES CXX)

message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
configure_file("${PROJECT_SOURCE_DIR}/include/version.h.in"
    "${PROJECT_SOURCE_DIR}/include/version.h")

option(SIMPLETEST_BUILD_TESTS "build tests alongside the project" OFF)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install")
set(BUILD_TESTING OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 缓存编译选项
set(CMAKE_CXX_STANDARD 23 CACHE INTERNAL "need cxx 23")
set(CMAKE_CXX_STANDARD_REQUIRED TRUE CACHE INTERNAL "required cxx 23")

message(STATUS "CMAKE_CXX_COMPILE_ID: ${CMAKE_CXX_COMPILER_ID}")
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_link_options(-stdlib=libc++)
endif()

#set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g")
SET(BUILD_SHARED_LIBS ON)

# 编译选项
add_compile_options(-stdlib=libc++)

# 更新 git 子模块
find_package(Git)
if (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if (NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
endif()

# 编译子模块
#set(GFLAGS_IS_A_DLL ON)
set(GFLAGS_BUILD_SHARED_LIBS ON)
set(GFLAGS_BUILD_STATIC_LIBS OFF)
set(GFLAGS_BUILD_gflags_LIB ON)
set(GFLAGS_BUILD_gflags_nothreads_LIB OFF)
#set(GFLAGS_INCLUDE_DIR "gflags")
set(GFLAGS_INSTALL_HEADERS ON)
set(GFLAGS_INSTALL_SHARED_LIBS ON)
set(GFLAGS_INSTALL_STATIC_LIBS ON)
set(GFLAGS_NAMESPACE "google")
set(GFLAGS_LIBRARY_INSTALL_DIR "lib64")
message(STATUS "compile gflags")
add_subdirectory(third_party/gflags)

set(gflags_DIR "${PROJECT_SOURCE_DIR}/install/lib64/cmake/gflags")
#set(gflags_DIR "${CMAKE_BINARY_DIR}/third_party/gflags")
#find_package(gflags REQUIRED)

set(WITH_GTEST OFF)
set(WITH_UNWIND OFF)
message(STATUS "compile glog")
set(BUILD_TESTING OFF)
set(WITH_GFLAGS ON)
set(WITH_CUSTOM_PREFIX OFF)
set(WITH_SYMBOLIZE OFF)
set(WITH_UNWIND OFF)
set(WITH_TLS OFF)
add_subdirectory(third_party/glog)
#target_link_libraries(glog PUBLIC gflags::gflags)
#target_link_libraries (glog
#    PRIVATE gflags::gflags)

# mimalloc设置
set(MI_BUILD_SHARED ON)
set(MI_BUILD_STATIC OFF)
set(MI_BUILD_OBJECT OFF)
cmake_dependent_option(MI_BUILD_TESTS "Build mimalloc tests" ON "BUILD_TESTING" OFF)
if (ENABLE_ASAN)
    cmake_dependent_option(MI_TRACK_ASAN "Build mimalloc address sanitizer" ON "ENABLE_ASAN" OFF)
    cmake_dependent_option(MI_DEBUG_TSAN "Build mimalloc thread sanitizer" OFF "MI_TRACK_ASAN" ON)
    cmake_dependent_option(MI_DEBUG_UBSAN "Build mimalloc undefined-behavior sanitizer" ON "ENABLE_ASAN" OFF)
endif()
add_subdirectory(third_party/mimalloc)

# 生成目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# 生成libdb
file(GLOB_RECURSE DBPOOL_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/dbpool/*.cpp")
message(STATUS "DBPOOL_SRC: ${DBPOOL_SRC}")
add_library(dbpool SHARED ${DBPOOL_SRC})

# include 目录
target_include_directories(
    dbpool
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

# 生成可执行文件
file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/simpletest/*.cpp")
message(STATUS "TEST_SRC: ${TEST_SRC}")
add_executable(${PROJECT_NAME} ${TEST_SRC})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_compile_options(
    ${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wno-sign-conversion
        -Wno-missing-field-initializers
        -Wshadow
        -Wnon-virtual-dtor
        -Wcast-align
        -Wfatal-errors
        -Wextra-semi
        )

# 依赖
target_link_libraries (${PROJECT_NAME}
    PUBLIC dbpool
    PUBLIC glog::glog
    PUBLIC gflags::gflags
    PUBLIC mimalloc)

if (SIMPLETEST_BUILD_TESTS)
    include(CTest)
    find_package(GTest QUIET)
    if (NOT GTEST_FOUND)
    message("GoogleTest not found. Consider installing it on your system. Downloading it from source...")
    include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_SHALLOW true
        )
        set(BUILD_GMOCK OFF CACHE BOOL "")
        FetchContent_GetProperties(googletest)
        if(NOT googletest_POPULATED)
            FetchContent_Populate(googletest)
            add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
        endif()
    endif()
    add_subdirectory(tests)
endif()

#INSTALL(TARGETS ${PROJECT_NAME} glog
#    RUNTIME DESTINATION bin
#    LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib)

# 安装头文件
#INSTALL(DIRECTORY include/ DESTINATION include/${PROJECT_NAME} FILES_MATCHING PATTERN "*.h")
