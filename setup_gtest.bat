@echo off
setlocal

:: setup_gtest.bat - 自动下载并配置 Google Test (Windows 版本)

:: --- 配置 ---
set "DOWNLOAD_URL=https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz"
set "ARCHIVE_FILE=gtest.tar.gz"
set "EXTRACTED_DIR=googletest-1.17.0"
set "TARGET_DIR=include\gtest"

:: --- 主逻辑 ---

:: 确保在项目根目录运行
if not exist "include" (
    echo [ERROR] 请在根目录下运行此脚本。
    exit /b 1
)
if not exist "CMakeLists.txt" (
    echo [ERROR] 请在根目录下运行此脚本。
    exit /b 1
)

:: 检查目标目录是否已存在
if exist "%TARGET_DIR%\" (
    echo [INFO] 目录 '%TARGET_DIR%' 已存在。
    set /p choice="是否要删除并重新下载? (y/N): "
    if /i not "%choice%"=="y" (
        echo [INFO] 操作取消。
        exit /b 0
    )
    echo [INFO] 正在删除旧的 gtest 目录...
    rmdir /s /q "%TARGET_DIR%"
    if errorlevel 1 (
        echo [ERROR] 删除 '%TARGET_DIR%' 失败。
        exit /b 1
    )
)

:: 下载 gtest
echo [INFO] 正在从 %DOWNLOAD_URL% 下载 Google Test...
powershell -Command "Invoke-WebRequest -Uri '%DOWNLOAD_URL%' -OutFile '%ARCHIVE_FILE%'"
if errorlevel 1 (
    echo [ERROR] 下载失败。请检查网络连接或 URL。
    if exist "%ARCHIVE_FILE%" del "%ARCHIVE_FILE%"
    exit /b 1
)

:: 解压缩
echo [INFO] 正在解压缩 '%ARCHIVE_FILE%'...
tar -xzf "%ARCHIVE_FILE%"
if errorlevel 1 (
    echo [ERROR] 解压缩失败。请确保已安装 tar 工具 (Windows 10/11 内置)。
    del "%ARCHIVE_FILE%"
    exit /b 1
)

:: 检查解压后的目录是否存在
if not exist "%EXTRACTED_DIR%\" (
    echo [ERROR] 解压后的目录 '%EXTRACTED_DIR%' 未找到。
    del "%ARCHIVE_FILE%"
    exit /b 1
)

:: 移动并重命名
echo [INFO] 正在将 '%EXTRACTED_DIR%' 移动到 '%TARGET_DIR%'...
move "%EXTRACTED_DIR%" "%TARGET_DIR%"
if errorlevel 1 (
    echo [ERROR] 移动目录失败。
    del "%ARCHIVE_FILE%"
    rmdir /s /q "%EXTRACTED_DIR%" 2>nul
    exit /b 1
)

:: 清理
echo [INFO] 正在清理临时文件...
del "%ARCHIVE_FILE%"

echo [INFO] Google Test 配置成功！

endlocal
