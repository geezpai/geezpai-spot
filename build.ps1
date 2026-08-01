param(
    [string]$Board = "nrf52840dk/nrf52840",
    [string]$BuildDir = (Join-Path $PSScriptRoot "..\build_ref"),
    [switch]$Pristine
)

$ErrorActionPreference = "Continue"

# NCS 工具链路径（与 nrf52840_dk 项目一致）
$toolchainRoot = "C:\ncs\toolchains\936afb6332"
$zephyrBase = "C:\ncs\v3.3.0\zephyr"
$zephyrSdk = Join-Path $toolchainRoot "opt\zephyr-sdk"
$pythonExe = Join-Path $toolchainRoot "opt\bin\python.exe"
$appDir = $PSScriptRoot

if (-not (Test-Path $pythonExe)) {
    throw "未找到 NCS Python: $pythonExe"
}

if (-not (Test-Path $zephyrBase)) {
    throw "未找到 Zephyr 基础目录: $zephyrBase"
}

$env:ZEPHYR_BASE = $zephyrBase
$env:ZEPHYR_SDK_INSTALL_DIR = $zephyrSdk
$env:PATH = "$(Join-Path $toolchainRoot 'opt\bin');$(Join-Path $toolchainRoot 'bin');$env:PATH"

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
