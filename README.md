# TinySTL

基于 C++11 标准实现的微型 STL 

## 项目结构

```
.
├── include/
│   ├── mystl/      # TinySTL 核心源码
│   └── gtest/      # Google Test 测试框架 (需通过脚本下载)
├── test/           # 单元测试代码
├── note/           # C++ 与 STL 学习笔记
├── CMakeLists.txt  # 主构建脚本
├── setup_gtest.sh  # Google Test 下载与配置脚本
└── README.md       
```

## 构建与测试

项目的构建依赖于 CMake，单元测试则使用 Google Test 框架。

### 步骤 1: 配置 Google Test

在开始构建之前，你需要先下载并配置 Google Test。项目提供了一个便捷的脚本来完成此操作。

在项目根目录下运行：

```bash
bash setup_gtest.sh
```

该脚本会自动从 GitHub 下载 Google Test，并将其放置在 `include/gtest` 目录下。

### 步骤 2: 使用 CMake 构建项目

完成 Google Test 的配置后，你可以使用标准的 CMake 流程来编译项目和测试。

```bash
cmake -B build
cmake --build build 
```

### 步骤 3: 运行测试

编译成功后，测试可执行文件会生成在 `build/test` 目录下。

```bash
# 在 build 目录下执行
./test/unit_test
```