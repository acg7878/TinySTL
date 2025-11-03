# include/CMakeLists.txt

# -----------------------------
# TinySTL 头文件库
# -----------------------------
add_library(TinySTL INTERFACE)

# 添加 mystl 头文件路径
target_include_directories(TinySTL INTERFACE
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

set(gtest_force_shared_crt ON)  # 强制 gtest 使用共享 (DLL) 运行时库
add_subdirectory(gtest)
# gtest 库会生成 gtest 和 gtest_main targets
