# 设置要求的 CMake 最低版本
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(MyTinySTL)

# 设置 C++ 标准为 C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- MyTinySTL 库 ---
# 这是一个 INTERFACE 库，因为它只有头文件
# INTERFACE 库本身不编译任何源文件，但可以指定属性，比如包含目录
add_library(${PROJECT_NAME} INTERFACE)

# 将 "include" 目录添加到 ${PROJECT_NAME} 目标的公有头文件搜索路径下
# 任何链接到 MyTinySTL 的目标都会自动获得这个 include 路径
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)


# --- 用于 IDE 上下文的虚拟目标 ---
# 这个可执行文件用于为 clangd 等工具提供编译信息
# 它包含了项目的所有头文件，以便工具可以正确解析它们
add_executable(ide_context dev/all_headers.cpp)

# 链接 MyTinySTL 库，这样 ide_context 就会继承它的 include 路径

target_link_libraries(ide_context PRIVATE ${PROJECT_NAME})

# 再次为 ide_context 明确指定 include 目录，以确保 clangd 能找到
target_include_directories(ide_context PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
