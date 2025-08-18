# include/CMakeLists.txt

# -----------------------------
# TinySTL 头文件库
# -----------------------------
add_library(MyTinySTL INTERFACE)

# 添加 mystl 头文件路径
target_include_directories(MyTinySTL INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# -----------------------------
# Google Test 头文件库（INTERFACE）
# -----------------------------
add_library(GTestHeaders INTERFACE)

# 添加 gtest include 目录
target_include_directories(GTestHeaders INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest/include
)

# -----------------------------
# 可选：编译 gtest 源码生成库
# 如果你希望 gtest 被编译链接
# -----------------------------
add_subdirectory(gtest)
# gtest 库会生成 gtest 和 gtest_main targets
