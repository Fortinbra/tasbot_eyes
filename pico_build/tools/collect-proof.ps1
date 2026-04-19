param(
    [string]$PicoSdkPath = "",
    [string]$Board = "pimoroni_plasma2350",
    [string]$BuildDir = "C:\ws\tasbot_eyes\pico_build\build\ws2812-proof",
    [string]$SerialPort = "",
    [int]$CaptureSeconds = 6
)

$ErrorActionPreference = "Stop"

function Resolve-FirstPath {
    param(
        [string[]]$Candidates,
        [string]$CommandName
    )

    foreach ($candidate in $Candidates) {
        if ($candidate -and (Test-Path $candidate)) {
            return $candidate
        }
    }

    $command = Get-Command $CommandName -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    throw "Could not resolve $CommandName."
}

$repoRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$cmake = Resolve-FirstPath @(
    "C:\Users\thegu\.pico-sdk\cmake\v3.31.5\bin\cmake.exe",
    "C:\Users\thegu\.pico-sdk\cmake\v3.29.9\bin\cmake.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Preview\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
) "cmake.exe"
$ninja = Resolve-FirstPath @(
    "C:\Users\thegu\.pico-sdk\ninja\v1.12.1\ninja.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Preview\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
) "ninja.exe"
$toolchainBin = Split-Path (Resolve-FirstPath @(
    "C:\Users\thegu\.pico-sdk\toolchain\14_2_Rel1\bin\arm-none-eabi-gcc.exe",
    "C:\Program Files\Arm\GNU Toolchain mingw-w64-x86_64-arm-none-eabi\bin\arm-none-eabi-gcc.exe"
) "arm-none-eabi-gcc.exe") -Parent
$picotool = Resolve-FirstPath @(
    "C:\Users\thegu\.pico-sdk\picotool\2.2.0\picotool\picotool.exe",
    "C:\Users\thegu\.pico-sdk\picotool\2.1.1\picotool\picotool.exe"
) "picotool.exe"

if (-not $PicoSdkPath) {
    if ($env:PICO_SDK_PATH) {
        $PicoSdkPath = $env:PICO_SDK_PATH
    } elseif (Test-Path "C:\ws\pico-sdk\pico_sdk_init.cmake") {
        $PicoSdkPath = "C:\ws\pico-sdk"
    } else {
        throw "PICO_SDK_PATH not provided and default C:\ws\pico-sdk was not found."
    }
}

$env:Path = "$toolchainBin;$(Split-Path $cmake -Parent);$(Split-Path $ninja -Parent);$(Split-Path $picotool -Parent);$env:Path"

if (Test-Path $BuildDir) {
    Remove-Item $BuildDir -Recurse -Force
}

Write-Host "== Configure =="
& $cmake -G Ninja -S (Join-Path $repoRoot "pico_build") -B $BuildDir "-DPICO_SDK_PATH=$PicoSdkPath" "-DPICO_BOARD=$Board"
if ($LASTEXITCODE -ne 0) {
    throw "Pico configure failed."
}

Write-Host "== Build =="
& $cmake --build $BuildDir
if ($LASTEXITCODE -ne 0) {
    throw "Pico build failed."
}

Write-Host "== Artifacts =="
Get-ChildItem $BuildDir -Filter "tasbot_eyes_pico.*" -File |
    Sort-Object Name |
    Select-Object Name, Length |
    Format-Table -AutoSize

Write-Host "== Artifact SHA256 =="
Get-ChildItem $BuildDir -Filter "tasbot_eyes_pico.*" -File |
    Sort-Object Name |
    Get-FileHash -Algorithm SHA256 |
    Select-Object Path, Hash |
    Format-Table -Wrap

Write-Host "== Ready banner in ELF =="
& "$env:SystemRoot\System32\findstr.exe" /C:"tasbot_eyes pico_build booting" /C:"tasbot_eyes pico_build ready" /C:"board contract: WS2812B on GPIO15, active 8x28 (224 pixels) within physical 8x32 (256 pixels)" /C:"runtime seam: logical 28x8 active frame -> column-serpentine mapper -> 256 LED physical transport" (Join-Path $BuildDir "tasbot_eyes_pico.elf")
if ($LASTEXITCODE -ne 0) {
    throw "Expected firmware banners and board contract were not found in the ELF."
}

$ports = Get-CimInstance Win32_SerialPort | Select-Object DeviceID, Description
Write-Host "== Serial ports =="
if ($ports) {
    $ports | Format-Table -AutoSize
} else {
    Write-Host "No serial ports detected."
}

if ($SerialPort) {
    Write-Host "== Serial capture from $SerialPort =="
    $serial = [System.IO.Ports.SerialPort]::new($SerialPort, 115200, [System.IO.Ports.Parity]::None, 8, [System.IO.Ports.StopBits]::One)
    $serial.ReadTimeout = 250
    $serial.Open()

    try {
        $deadline = (Get-Date).AddSeconds($CaptureSeconds)
        while ((Get-Date) -lt $deadline) {
            try {
                $line = $serial.ReadLine()
                if ($line) {
                    Write-Host $line
                }
            } catch [System.TimeoutException] {
            }
        }
    } finally {
        $serial.Close()
        $serial.Dispose()
    }
} else {
    Write-Host "== Serial capture =="
    Write-Host "Skipped. Pass -SerialPort COMx to capture the boot-ready window on hardware."
}
