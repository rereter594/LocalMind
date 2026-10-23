cmake_minimum_required(VERSION 3.22.1)
project("localmind")

# مسار كود llama.cpp
set(LLAMA_DIR ${CMAKE_SOURCE_DIR}/../../../../../llama.cpp)

# إضافة مجلدات الرأس (Headers)
include_directories(${LLAMA_DIR})
include_directories(${LLAMA_DIR}/ggml/include)

# تجميع مكتبة الجسر (JNI)
add_library(localmind SHARED llama_bridge.cpp)

# ربط مكتبة llama_common و ggml
find_library(log-lib log)
target_link_libraries(localmind ${log-lib} android)

# تضمين كود llama.cpp مباشرة (لأننا سنستخدم CMake الخاص به)
add_subdirectory(${LLAMA_DIR} ${CMAKE_BINARY_DIR}/llama.cpp)
target_link_libraries(localmind llama ggml)
