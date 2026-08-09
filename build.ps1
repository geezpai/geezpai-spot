param(
    [string]$Board = "nrf52840dk/nrf52840",
    [string]$BuildDir = (Join-Path $PSScriptRoot "..\build_ref"),
    [switch]$Pristine
)

$ErrorActionPreference = "Continue"

# NCS 工具链路径 — 按优先级自动检测:
#   1. 环境变量 NCS_TOOLCHAIN_PATH / ZEPHYR_BASE
#   2. nRF Connect for Desktop Toolchain Manager 默认路径
#   3. 常见自定义路径
if (-not $env:ZEPHYR_BASE) {
    $candidates = @(
        "C:\ncs\v3.3.0\zephyr",
        "$env:USERPROFILE\ncs\v3.3.0\zephyr"
    )
    foreach ($c in $candidates) {
        if (Test-Path $c) { $env:ZEPHYR_BASE = $c; break }
    }
}
if (-not $env:ZEPHYR_BASE) {
    throw "未找到 Zephyr。请设置 ZEPHYR_BASE 环境变量，或通过 nRF Connect Toolchain Manager 安装 NCS v3.3.0"
}
$zephyrBase = $env:ZEPHYR_BASE

# 自动查找工具链 (在 Zephyr SDK 的上级目录)
if (-not $env:ZEPHYR_SDK_INSTALL_DIR) {
    $sdkCandidates = @(
        "C:\ncs\toolchains\936afb6332\opt\zephyr-sdk",
        "$env:USERPROFILE\ncs\toolchains\*\opt\zephyr-sdk"
    )
    foreach ($c in $sdkCandidates) {
        $resolved = (Resolve-Path $c -ErrorAction SilentlyContinue | Select-Object -First 1)
        if ($resolved) { $env:ZEPHYR_SDK_INSTALL_DIR = $resolved.Path; break }
    }
}
$zephyrSdk = $env:ZEPHYR_SDK_INSTALL_DIR

# 查找 NCS Python (在 Zephyr SDK 的 ../bin/python.exe)
$toolchainBin = Join-Path (Split-Path $zephyrSdk -Parent) "bin"
$pythonExe = Join-Path $toolchainBin "python.exe"
if (-not (Test-Path $pythonExe)) {
    throw "未找到 NCS Python: $pythonExe。请设置 ZEPHYR_SDK_INSTALL_DIR 环境变量。"
}
$env:PATH = "$toolchainBin;$(Join-Path (Split-Path $zephyrSdk -Parent) 'bin');$env:PATH"

$appDir = $PSScriptRoot

$env:ZEPHYR_BASE = $zephyrBase
$env:ZEPHYR_SDK_INSTALL_DIR = $zephyrSdk

# 官方 locator_tag 样例使用 sysbuild（包含 MCUboot + 主应用）
$westArgs = @(
    "-m",
    "west",
    "-z",
    $zephyrBase,
    "build",
    "--sysbuild",
    "-s",
    $appDir,
    "-b",
    $Board,
    "-d",
    $BuildDir
)

if ($Pristine) {
    $westArgs += @("-p", "always")
}

& $pythonExe @westArgs

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
