param(
    [string]$BuildDir = (Join-Path $PSScriptRoot "..\build_ref"),
    [ValidateSet("auto", "nrfjprog", "jlink")]
    [string]$Runner = "auto",
    [string]$DevId,
    [switch]$Erase,
    [switch]$Recover
)

# 不使用 Stop —— python/west 把警告写到 stderr 时会被误判为错误中断
$ErrorActionPreference = "Continue"

# NCS 工具链 — 自动检测 (也可通过环境变量覆盖)
if (-not $env:ZEPHYR_BASE) {
    $candidates = @("C:\ncs\v3.3.0\zephyr", "$env:USERPROFILE\ncs\v3.3.0\zephyr")
    foreach ($c in $candidates) { if (Test-Path $c) { $env:ZEPHYR_BASE = $c; break } }
}
if (-not $env:ZEPHYR_BASE) { throw "未找到 Zephyr。请设置 ZEPHYR_BASE 环境变量。" }
$zephyrBase = $env:ZEPHYR_BASE

if (-not $env:ZEPHYR_SDK_INSTALL_DIR) {
    $sdkCandidates = @("C:\ncs\toolchains\936afb6332\opt\zephyr-sdk")
    foreach ($c in $sdkCandidates) { if (Test-Path $c) { $env:ZEPHYR_SDK_INSTALL_DIR = $c; break } }
}
$zephyrSdk = $env:ZEPHYR_SDK_INSTALL_DIR
$toolchainBin = Join-Path (Split-Path $zephyrSdk -Parent) "bin"
$pythonExe = Join-Path $toolchainBin "python.exe"
$env:PATH = "$toolchainBin;$(Join-Path (Split-Path $zephyrSdk -Parent) 'bin');$env:PATH"

# J-Link — PATH 中找，失败则尝试常见路径
$jlinkCommander = (Get-Command JLink.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source -First 1)
if (-not $jlinkCommander) {
    $jlinkFallbacks = @(
        "C:\Program Files\SEGGER\JLink\JLink.exe",
        "C:\Program Files (x86)\SEGGER\JLink\JLink.exe"
    )
    foreach ($c in $jlinkFallbacks) { if (Test-Path $c) { $jlinkCommander = $c; break } }
}

if (-not (Test-Path $jlinkCommander)) {
    $jlinkFromPath = (Get-Command JLink.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source -First 1)
    if ($jlinkFromPath) {
        $jlinkCommander = $jlinkFromPath
    } else {
        foreach ($candidate in $jlinkCommanderFallbacks) {
            if (Test-Path $candidate) {
                $jlinkCommander = $candidate
                break
            }
        }
    }
}

$env:ZEPHYR_BASE = $zephyrBase
$env:ZEPHYR_SDK_INSTALL_DIR = $zephyrSdk
$env:PATH = "$(Join-Path $toolchainRoot 'opt\bin');$(Join-Path $toolchainRoot 'bin');$env:PATH"

function Invoke-JLinkSysbuildFlash {
    param(
        [string]$BuildDirPath,
        [string]$CommanderPath,
        [string]$JLinkSerial,
        [switch]$DoRecover
    )

    $imagePaths = @(
        (Join-Path $BuildDirPath "mcuboot\zephyr\zephyr.hex"),
        (Join-Path $BuildDirPath "nrf52840-dk_nearby_slot1_variant\zephyr\zephyr.signed.hex"),
        (Join-Path $BuildDirPath "modules\nrf\subsys\bluetooth\fast_pair\fp_provisioning_data.hex"),
        (Join-Path $BuildDirPath "nrf52840-dk_nearby\zephyr\zephyr.signed.hex")
    )

    foreach ($imagePath in $imagePaths) {
        if (-not (Test-Path $imagePath)) {
            throw "未找到镜像文件: $imagePath"
        }
    }

    $scriptLines = @(
        "device nRF52840_xxAA",
        "si SWD",
        "speed 4000"
    )

    if ($JLinkSerial) {
        $scriptLines += "SelectEmuBySN $JLinkSerial"
    }

    $scriptLines += "connect"
    $scriptLines += "r"
    if ($DoRecover) {
        $scriptLines += "erase"
    }

    foreach ($imagePath in $imagePaths) {
        $scriptLines += "loadfile $imagePath"
    }

    $scriptLines += @(
        "r",
        "g",
        "q"
    )

    $jlinkScript = Join-Path $env:TEMP "jlink_sysbuild_flash.jlink"
    Set-Content -Path $jlinkScript -Value $scriptLines -Encoding ASCII

    & $CommanderPath -CommanderScript $jlinkScript
    if ($LASTEXITCODE -ne 0) {
        throw "J-Link 烧录失败，退出码: $LASTEXITCODE"
    }
}

# 自动选择可用的 runner（优先 nrfjprog，其次 J-Link）
if ($Runner -eq "auto") {
    if (Get-Command nrfjprog -ErrorAction SilentlyContinue) {
        $Runner = "nrfjprog"
    } elseif (Test-Path $jlinkCommander) {
        $Runner = "jlink"
    } else {
        throw "未找到 nrfjprog 或 J-Link，请安装 nRF Command Line Tools 或 SEGGER J-Link"
    }
}

$westArgs = @(
    "-m", "west",
    "-z", $zephyrBase,
    "flash",
    "-d", $BuildDir,
    "--no-rebuild",
    "-r", $Runner
)

$runnerArgs = @()
if ($DevId)   { $runnerArgs += @("--dev-id", $DevId) }
if ($Erase)   { $runnerArgs += "--erase" }
if ($Recover) { $runnerArgs += "--recover" }
if ($Runner -eq "jlink") {
    if (-not (Test-Path $jlinkCommander)) {
        throw "未找到 J-Link Commander: $jlinkCommander"
    }
    $runnerArgs += @("--commander", $jlinkCommander)
}

if ($runnerArgs.Count -gt 0) {
    $westArgs += "--"
    $westArgs += $runnerArgs
}

Write-Host "Runner: $Runner"
Write-Host "BuildDir: $BuildDir"

if ($Runner -eq "jlink") {
    if (-not (Test-Path $jlinkCommander)) {
        throw "未找到 J-Link Commander: $jlinkCommander"
    }

    Invoke-JLinkSysbuildFlash -BuildDirPath $BuildDir -CommanderPath $jlinkCommander -JLinkSerial $DevId -DoRecover:$Recover
    exit 0
}

& $pythonExe @westArgs
exit $LASTEXITCODE
