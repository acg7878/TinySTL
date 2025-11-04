# PowerShell 脚本 - 自动下载并配置 Google Test

# --- 配置 ---
$DownloadUrl = "https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz"
$ArchiveFile = "gtest.tar.gz"
$ExtractedDir = "googletest-1.17.0"
$TargetDir = "include\gtest"

# --- 主逻辑 ---

# 设置 PowerShell 在遇到错误时停止执行
$ErrorActionPreference = "Stop"

# 确保在项目根目录运行
if (-not (Test-Path "include") -or -not (Test-Path "CMakeLists.txt")) {
    Write-Error "[错误] 请在 TinySTL 项目的根目录下运行此脚本。"
    exit 1
}

# 检查目标目录是否已存在
if (Test-Path $TargetDir) {
    Write-Host "[信息] 目录 '$TargetDir' 已存在。"
    $choice = Read-Host "是否要删除并重新下载? (y/N)"
    if ($choice -ne 'y') {
        Write-Host "[信息] 操作取消。"
        exit 0
    }
    
    Write-Host "[信息] 正在删除旧的 gtest 目录..."
    try {
        Remove-Item -Recurse -Force $TargetDir
    } catch {
        Write-Error "[错误] 删除 '$TargetDir' 失败。 $_"
        exit 1
    }
}

try {
    # 下载 gtest
    Write-Host "[信息] 正在从 $DownloadUrl 下载 Google Test..."
    Invoke-WebRequest -Uri $DownloadUrl -OutFile $ArchiveFile

    # 解压缩
    Write-Host "[信息] 正在解压缩 '$ArchiveFile'..."
    tar -xzf $ArchiveFile

    # 检查解压后的目录是否存在
    if (-not (Test-Path $ExtractedDir)) {
        throw "解压后的目录 '$ExtractedDir' 未找到。"
    }

    # 移动并重命名
    Write-Host "[信息] 正在将 '$ExtractedDir' 移动到 '$TargetDir'..."
    Move-Item -Path $ExtractedDir -Destination $TargetDir

    Write-Host "[成功] Google Test 配置成功！"

} catch {
    Write-Error "[错误] 操作失败: $($_.Exception.Message)"
    exit 1
} finally {
    # 清理
    if (Test-Path $ArchiveFile) {
        Write-Host "[信息] 正在清理临时文件..."
        Remove-Item $ArchiveFile
    }
}
