cmake_minimum_required(VERSION 3.10)
project(TinySTL VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 如果是 MSVC 编译器，添加 /utf-8 选项以支持 UTF-8 源文件
if(MSVC)
  add_compile_options(/utf-8)
endif()

# 添加 include 子目录（创建 INTERFACE 库）
add_subdirectory(include)

# 测试
enable_testing()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test")
    add_subdirectory(test)
endif()

# examples
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    add_subdirectory(examples)
endif()
