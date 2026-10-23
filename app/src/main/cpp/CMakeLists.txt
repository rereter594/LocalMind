cmake_minimum_required(VERSION 3.22.1)
project("localmind")

get_filename_component(LLAMA_DIR "${CMAKE_SOURCE_DIR}/../../../../llama.cpp" ABSOLUTE)

if(NOT EXISTS ${LLAMA_DIR})
    message(FATAL_ERROR "llama.cpp not found at ${LLAMA_DIR}")
endif()

include_directories(${LLAMA_DIR})
include_directories(${LLAMA_DIR}/ggml/include)
include_directories(${LLAMA_DIR}/include)
include_directories(${LLAMA_DIR}/common)

add_library(localmind SHARED llama_bridge.cpp)

find_library(log-lib log)
target_link_libraries(localmind ${log-lib} android)

# إعدادات هامة لتشغيل llama.cpp على الأندرويد
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_CURL OFF CACHE BOOL "" FORCE)
set(GGML_OPENMP OFF CACHE BOOL "" FORCE)
set(GGML_LLAMAFILE OFF CACHE BOOL "" FORCE)
set(GGML_ACCELERATE OFF CACHE BOOL "" FORCE)

add_subdirectory(${LLAMA_DIR} ${CMAKE_BINARY_DIR}/llama.cpp)

target_link_libraries(localmind llama ggml)
