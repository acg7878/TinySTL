#!/bin/bash

# setup_gtest.sh - 自动下载并配置 Google Test

# --- 配置 ---
DOWNLOAD_URL="https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz"
ARCHIVE_FILE="gtest.tar.gz"
EXTRACTED_DIR="googletest-1.17.0"
TARGET_DIR="include/gtest"

# --- 函数 ---
# 打印信息
info() {
    echo "[INFO] $1"
}

# 打印错误并退出
error() {
    echo "[ERROR] $1" >&2
    exit 1
}

# --- 主逻辑 ---
# 确保在项目根目录运行
if [ ! -d "include" ] || [ ! -f "CMakeLists.txt" ]; then
    error "请在 MyTinySTL 项目的根目录下运行此脚本。"
fi

# 检查目标目录是否已存在
if [ -d "$TARGET_DIR" ]; then
    info "目录 '$TARGET_DIR' 已存在。"
    read -p "是否要删除并重新下载? (y/N): " choice
    case "$choice" in
      y|Y )
        info "正在删除旧的 gtest 目录..."
        rm -rf "$TARGET_DIR" || error "删除 '$TARGET_DIR' 失败。"
        ;;
      * )
        info "操作取消。"
        exit 0
        ;;
    esac
fi

# 下载 gtest
info "正在从 $DOWNLOAD_URL 下载 Google Test..."
curl -L "$DOWNLOAD_URL" -o "$ARCHIVE_FILE" || error "下载失败。请检查网络连接或 URL。"

# 解压缩
info "正在解压缩 '$ARCHIVE_FILE'..."
tar -xzf "$ARCHIVE_FILE" || error "解压缩失败。请确保已安装 tar 工具。"

# 检查解压后的目录是否存在
if [ ! -d "$EXTRACTED_DIR" ]; then
    error "解压后的目录 '$EXTRACTED_DIR' 未找到。"
fi

# 移动并重命名
info "正在将 '$EXTRACTED_DIR' 移动到 '$TARGET_DIR'..."
mv "$EXTRACTED_DIR" "$TARGET_DIR" || error "移动目录失败。"

# 清理
info "正在清理临时文件..."
rm "$ARCHIVE_FILE"

info "Google Test 配置成功！"
