param(
    [string]$ProjectRoot = "",
    [string]$AssetName = "colorful.gif",
    [string]$SymbolPrefix = "",
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if (-not $SymbolPrefix) {
    $SymbolPrefix = [System.IO.Path]::GetFileNameWithoutExtension($AssetName) -replace '[^a-zA-Z0-9]', '_' -replace '_+', '_'
    $SymbolPrefix = $SymbolPrefix.ToLowerInvariant().Trim('_')
}
$upperPrefix = $SymbolPrefix.ToUpperInvariant()

if (-not $ProjectRoot) {
    $ProjectRoot = Split-Path $PSScriptRoot -Parent
}

if (-not $OutputDir) {
    $OutputDir = Join-Path $ProjectRoot "assets\generated"
}

$repoRoot = if (Test-Path (Join-Path $ProjectRoot "external")) {
    $ProjectRoot
} else {
    Split-Path $ProjectRoot -Parent
}

function ConvertTo-CString {
    param([string]$Value)

    return $Value.Replace('\', '\\').Replace('"', '\"')
}

function Get-Sha256Hex {
    param([string]$Path)

    $stream = [System.IO.File]::OpenRead($Path)
    try {
        $sha256 = [System.Security.Cryptography.SHA256]::Create()
        try {
            return ([System.BitConverter]::ToString($sha256.ComputeHash($stream))).Replace("-", "")
        } finally {
            $sha256.Dispose()
        }
    } finally {
        $stream.Dispose()
    }
}

function Get-RepoRelativePath {
    param(
        [string]$Root,
        [string]$Path
    )

    $rootPath = [System.IO.Path]::GetFullPath($Root)
    if (-not $rootPath.EndsWith("\")) {
        $rootPath += "\"
    }

    $rootUri = New-Object System.Uri($rootPath)
    $pathUri = New-Object System.Uri([System.IO.Path]::GetFullPath($Path))

    return [System.Uri]::UnescapeDataString($rootUri.MakeRelativeUri($pathUri).ToString()).Replace('/', '\')
}

function Get-CandidateRank {
    param(
        [string]$PoolRoot,
        [System.IO.FileInfo]$Candidate,
        [string]$RequestedAsset
    )

    $relative = (Get-RepoRelativePath -Root $PoolRoot -Path $Candidate.FullName).ToLowerInvariant()
    $asset = $RequestedAsset.ToLowerInvariant()

    switch ($relative) {
        $asset { return 0 }
        "others\$asset" { return 0 }
        "blinks\$asset" { return 0 }
        default {
            if ($relative.Contains("backup")) {
                return 50
            }

            return 10
        }
    }
}

function Get-AssetCandidates {
    param(
        [string]$PoolRoot,
        [string]$RequestedAsset
    )

    if (-not (Test-Path $PoolRoot)) {
        return @()
    }

    return @(Get-ChildItem -Path $PoolRoot -Filter $RequestedAsset -Recurse -File | Sort-Object FullName)
}

function Select-AssetCandidate {
    param(
        [object[]]$Candidates,
        [string]$PoolRoot,
        [string]$PoolName,
        [string]$RequestedAsset
    )

    $candidateList = @($Candidates | Where-Object { $_ -is [System.IO.FileInfo] })

    if ($candidateList.Count -eq 0) {
        return $null
    }

    $ranked = foreach ($candidate in $candidateList) {
        [PSCustomObject]@{
            Candidate = $candidate
            Relative = Get-RepoRelativePath -Root $PoolRoot -Path $candidate.FullName
            Rank = Get-CandidateRank -PoolRoot $PoolRoot -Candidate $candidate -RequestedAsset $RequestedAsset
        }
    }

    $ordered = @($ranked | Sort-Object Rank, Relative)
    $best = $ordered[0]
    $ties = @($ordered | Where-Object { $_.Rank -eq $best.Rank })

    if ($ties.Count -gt 1) {
        throw "Ambiguous $PoolName candidates for ${RequestedAsset}: $($ties.Relative -join ', ')"
    }

    return $best
}

Add-Type -AssemblyName System.Drawing

$externalPoolRoot = Join-Path $repoRoot "external\TASBot-eye-animations\gifs"
$legacyPoolRoot = Join-Path $repoRoot "gifs"
$sourceRule = "prefer submodule, fall back to root gifs, fail loudly otherwise"

$externalCandidates = @(Get-AssetCandidates -PoolRoot $externalPoolRoot -RequestedAsset $AssetName)
$legacyCandidates = @(Get-AssetCandidates -PoolRoot $legacyPoolRoot -RequestedAsset $AssetName)
$externalSelection = Select-AssetCandidate -Candidates $externalCandidates -PoolRoot $externalPoolRoot -PoolName "submodule" -RequestedAsset $AssetName
$legacySelectionError = $null
$legacySelection = try {
    Select-AssetCandidate -Candidates $legacyCandidates -PoolRoot $legacyPoolRoot -PoolName "legacy" -RequestedAsset $AssetName
} catch {
    $legacySelectionError = $_.Exception.Message
    $null
}
$selected = $externalSelection

if ($null -eq $selected) {
    if ($null -ne $legacySelectionError) {
        throw "Could not select from legacy candidates for ${AssetName}: $legacySelectionError"
    }
    $selected = $legacySelection
}

if ($null -eq $selected) {
    throw "Could not find $AssetName in external\TASBot-eye-animations\gifs or gifs."
}

$selectedPoolRoot = if ($selected.Candidate.FullName.StartsWith($externalPoolRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
    $externalPoolRoot
} else {
    $legacyPoolRoot
}
$selectedPool = if ($selectedPoolRoot -eq $externalPoolRoot) {
    "external\TASBot-eye-animations\gifs"
} else {
    "gifs"
}
$selectedSource = Get-RepoRelativePath -Root $repoRoot -Path $selected.Candidate.FullName
$selectedHash = Get-Sha256Hex -Path $selected.Candidate.FullName

$comparisonState = if ($null -ne $legacySelectionError) {
    "legacy-ambiguous"
} elseif (@($externalCandidates).Count -gt 0 -and @($legacyCandidates).Count -gt 0) {
    $legacyHash = Get-Sha256Hex -Path $legacySelection.Candidate.FullName
    if ($legacyHash -eq $selectedHash) { "identical" } else { "different" }
} elseif (@($externalCandidates).Count -gt 0) {
    "submodule-only"
} else {
    "legacy-only"
}

$image = [System.Drawing.Image]::FromFile($selected.Candidate.FullName)
try {
    $frameDimension = [System.Drawing.Imaging.FrameDimension]::Time
    $frameCount = $image.GetFrameCount($frameDimension)
    $imageWidth = $image.Width
    $imageHeight = $image.Height

    if ($imageWidth -ne 28 -or $imageHeight -ne 8) {
        throw "$selectedSource is ${imageWidth}x${imageHeight}; expected 28x8 for the current logical contract."
    }

    $delayProperty = $null
    try {
        $delayProperty = $image.GetPropertyItem(0x5100)
    } catch {
    }

    $delayValues = New-Object "System.Collections.Generic.List[string]"
    $pixelLines = New-Object "System.Collections.Generic.List[string]"

    for ($frameIndex = 0; $frameIndex -lt $frameCount; ++$frameIndex) {
        [void]$image.SelectActiveFrame($frameDimension, $frameIndex)

        $bitmap = New-Object System.Drawing.Bitmap($imageWidth, $imageHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        try {
            $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
            try {
                $graphics.Clear([System.Drawing.Color]::Black)
                $graphics.DrawImage($image, 0, 0, $imageWidth, $imageHeight)
            } finally {
                $graphics.Dispose()
            }

            $delayMs = 100
            if ($delayProperty -and $delayProperty.Value.Length -ge (($frameIndex + 1) * 4)) {
                $rawDelay = [BitConverter]::ToInt32($delayProperty.Value, $frameIndex * 4)
                if ($rawDelay -gt 0) {
                    $delayMs = $rawDelay * 10
                }
            }

            $delayValues.Add(("{0}u" -f $delayMs))
            $pixelLines.Add(("    /* frame {0} */" -f $frameIndex))

            for ($y = 0; $y -lt $bitmap.Height; ++$y) {
                $rowValues = New-Object "System.Collections.Generic.List[string]"

                for ($x = 0; $x -lt $bitmap.Width; ++$x) {
                    $pixel = $bitmap.GetPixel($x, $y)
                    $rgb = if ($pixel.A -eq 0) {
                        0
                    } else {
                        (([uint32]$pixel.R -shl 16) -bor ([uint32]$pixel.G -shl 8) -bor [uint32]$pixel.B)
                    }

                    $rowValues.Add(("0x{0:X6}u" -f $rgb))
                }

                $pixelLines.Add(("    {0}," -f ($rowValues -join ", ")))
            }
        } finally {
            $bitmap.Dispose()
        }
    }
} finally {
    $image.Dispose()
}

$headerPath = Join-Path $OutputDir "${SymbolPrefix}_asset.generated.h"
$metadataPath = Join-Path $OutputDir "${SymbolPrefix}_asset.metadata.txt"

$header = New-Object System.Text.StringBuilder
[void]$header.AppendLine("#pragma once")
[void]$header.AppendLine("")
[void]$header.AppendLine('#include <stdint.h>')
[void]$header.AppendLine("")
[void]$header.AppendLine('#include "runtime_types.h"')
[void]$header.AppendLine("")
[void]$header.AppendLine(('// Generated by tools\generate-gif-asset.ps1. Do not hand-edit.'))
[void]$header.AppendLine(('#define TASBOT_{0}_ASSET_NAME "{1}"' -f $upperPrefix, (ConvertTo-CString $AssetName)))
[void]$header.AppendLine(('#define TASBOT_{0}_ASSET_SOURCE_PATH "{1}"' -f $upperPrefix, (ConvertTo-CString $selectedSource)))
[void]$header.AppendLine(('#define TASBOT_{0}_ASSET_SOURCE_POOL "{1}"' -f $upperPrefix, (ConvertTo-CString $selectedPool)))
[void]$header.AppendLine(('#define TASBOT_{0}_ASSET_SOURCE_RULE "{1}"' -f $upperPrefix, (ConvertTo-CString $sourceRule)))
[void]$header.AppendLine(('#define TASBOT_{0}_ASSET_SOURCE_SHA256 "{1}"' -f $upperPrefix, $selectedHash))
[void]$header.AppendLine(('#define TASBOT_{0}_ASSET_WIDTH {1}u' -f $upperPrefix, $imageWidth))
[void]$header.AppendLine(('#define TASBOT_{0}_ASSET_HEIGHT {1}u' -f $upperPrefix, $imageHeight))
[void]$header.AppendLine(('#define TASBOT_{0}_ASSET_FRAME_COUNT {1}u' -f $upperPrefix, $frameCount))
[void]$header.AppendLine("")
[void]$header.AppendLine(("static const uint16_t g_tasbot_${SymbolPrefix}_frame_delays_ms[TASBOT_${upperPrefix}_ASSET_FRAME_COUNT] = {{ {0} }};" -f ($delayValues -join ", ")))
[void]$header.AppendLine("")
[void]$header.AppendLine("static const tasbot_color_t g_tasbot_${SymbolPrefix}_pixels[TASBOT_${upperPrefix}_ASSET_FRAME_COUNT * TASBOT_${upperPrefix}_ASSET_WIDTH * TASBOT_${upperPrefix}_ASSET_HEIGHT] = {")
foreach ($line in $pixelLines) {
    [void]$header.AppendLine($line)
}
[void]$header.AppendLine("};")

$metadata = @(
    "asset_name=$AssetName"
    "selected_source=$selectedSource"
    "selected_pool=$selectedPool"
    "selected_sha256=$selectedHash"
    "source_rule=$sourceRule"
    "comparison_state=$comparisonState"
    ("external_candidates={0}" -f ((@($externalCandidates | ForEach-Object { Get-RepoRelativePath -Root $repoRoot -Path $_.FullName }) -join "; ")))
    ("legacy_candidates={0}" -f ((@($legacyCandidates | ForEach-Object { Get-RepoRelativePath -Root $repoRoot -Path $_.FullName }) -join "; ")))
    ("dimensions={0}x{1}" -f $imageWidth, $imageHeight)
    ("frame_count={0}" -f $frameCount)
    ("frame_delays_ms={0}" -f (($delayValues | ForEach-Object { $_.TrimEnd('u') }) -join ","))
) -join [Environment]::NewLine

[System.IO.Directory]::CreateDirectory($OutputDir) | Out-Null
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($headerPath, $header.ToString(), $utf8NoBom)
[System.IO.File]::WriteAllText($metadataPath, $metadata + [Environment]::NewLine, $utf8NoBom)
