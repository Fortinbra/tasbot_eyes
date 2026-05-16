param(
    [string]$ProjectRoot = "",
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if (-not $ProjectRoot) {
    $ProjectRoot = Split-Path $PSScriptRoot -Parent
}

if (-not $OutputDir) {
    $OutputDir = Join-Path $ProjectRoot "assets\generated"
}

$generateScript = Join-Path $PSScriptRoot "generate-gif-asset.ps1"

# Playlist order: all 21 animations
$animations = @(
    [PSCustomObject]@{ AssetName = "startup.gif";           SymbolPrefix = "startup" },
    [PSCustomObject]@{ AssetName = "base.gif";              SymbolPrefix = "base" },
    [PSCustomObject]@{ AssetName = "blink.gif";             SymbolPrefix = "blink" },
    [PSCustomObject]@{ AssetName = "colorful.gif";          SymbolPrefix = "colorful" },
    [PSCustomObject]@{ AssetName = "heart eyes.gif";        SymbolPrefix = "heart_eyes" },
    [PSCustomObject]@{ AssetName = "coin eyes.gif";         SymbolPrefix = "coin_eyes" },
    [PSCustomObject]@{ AssetName = "smile.gif";             SymbolPrefix = "smile" },
    [PSCustomObject]@{ AssetName = "smirk.gif";             SymbolPrefix = "smirk" },
    [PSCustomObject]@{ AssetName = "uwu.gif";               SymbolPrefix = "uwu" },
    [PSCustomObject]@{ AssetName = "disc.gif";              SymbolPrefix = "disc" },
    [PSCustomObject]@{ AssetName = "asteroids.gif";         SymbolPrefix = "asteroids" },
    [PSCustomObject]@{ AssetName = "portal_eyes.gif";       SymbolPrefix = "portal_eyes" },
    [PSCustomObject]@{ AssetName = "whirl.gif";             SymbolPrefix = "whirl" },
    [PSCustomObject]@{ AssetName = "loading.gif";           SymbolPrefix = "loading" },
    [PSCustomObject]@{ AssetName = "look left up.gif";      SymbolPrefix = "look_left_up" },
    [PSCustomObject]@{ AssetName = "look strangly up.gif";  SymbolPrefix = "look_strangly_up" },
    [PSCustomObject]@{ AssetName = "dead.gif";              SymbolPrefix = "dead" },
    [PSCustomObject]@{ AssetName = "file.gif";              SymbolPrefix = "file" },
    [PSCustomObject]@{ AssetName = "testbot.gif";           SymbolPrefix = "testbot" },
    [PSCustomObject]@{ AssetName = "gray.gif";              SymbolPrefix = "gray" },
    [PSCustomObject]@{ AssetName = "twink.gif";             SymbolPrefix = "twink" }
)

# Ensure explicit entries are unique before generation.
$seenAssetNames = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
$seenSymbolPrefixes = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
foreach ($anim in $animations) {
    if (-not $seenAssetNames.Add($anim.AssetName)) {
        throw "Duplicate AssetName in animation list: $($anim.AssetName)"
    }

    if (-not $seenSymbolPrefixes.Add($anim.SymbolPrefix)) {
        throw "Duplicate SymbolPrefix in animation list: $($anim.SymbolPrefix)"
    }
}

[System.IO.Directory]::CreateDirectory($OutputDir) | Out-Null

Write-Host "Generating $($animations.Count) animation assets..."

foreach ($anim in $animations) {
    Write-Host "  -> $($anim.SymbolPrefix) ($($anim.AssetName))"
    & $generateScript `
        -ProjectRoot $ProjectRoot `
        -AssetName $anim.AssetName `
        -SymbolPrefix $anim.SymbolPrefix `
        -OutputDir $OutputDir
    if (-not $?) {
        throw "generate-gif-asset.ps1 failed for $($anim.AssetName)"
    }
}

Write-Host "Writing animation_registry.generated.h..."

# Build playlist entries after filtering one-frame assets and duplicate content.
$playlistAnimations = New-Object System.Collections.Generic.List[object]
$seenSha256 = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
foreach ($anim in $animations) {
    $metadataPath = Join-Path $OutputDir "$($anim.SymbolPrefix)_asset.metadata.txt"
    if (-not (Test-Path -LiteralPath $metadataPath)) {
        throw "Missing metadata for $($anim.AssetName): $metadataPath"
    }

    $meta = @{}
    foreach ($line in Get-Content -LiteralPath $metadataPath) {
        if ($line -match '^(?<k>[^=]+)=(?<v>.*)$') {
            $meta[$Matches.k] = $Matches.v
        }
    }

    $frameCount = [int]$meta["frame_count"]
    if ($frameCount -le 1) {
        Write-Host "  -> Skipping one-frame animation: $($anim.AssetName)"
        continue
    }

    $sha = $meta["selected_sha256"]
    if ($sha -and -not $seenSha256.Add($sha)) {
        Write-Host "  -> Skipping duplicate animation content: $($anim.AssetName)"
        continue
    }

    [void]$playlistAnimations.Add($anim)
}

if ($playlistAnimations.Count -eq 0) {
    throw "No animations left after filtering (one-frame and duplicates)."
}

Write-Host "Playlist entries after filtering: $($playlistAnimations.Count)"

$registry = New-Object System.Text.StringBuilder
[void]$registry.AppendLine("#pragma once")
[void]$registry.AppendLine("")
[void]$registry.AppendLine("// Generated by tools\generate-animation-registry.ps1. Do not hand-edit.")
[void]$registry.AppendLine("")
[void]$registry.AppendLine('#include "embedded_animation.h"')
[void]$registry.AppendLine("")

# Include all per-asset headers
foreach ($anim in $playlistAnimations) {
    [void]$registry.AppendLine("#include `"$($anim.SymbolPrefix)_asset.generated.h`"")
}

[void]$registry.AppendLine("")

# Static animation struct instances
foreach ($anim in $playlistAnimations) {
    $lower = $anim.SymbolPrefix
    $upper = $lower.ToUpperInvariant()
    [void]$registry.AppendLine("static const tasbot_embedded_animation_t g_tasbot_${lower}_animation = {")
    [void]$registry.AppendLine("    .name          = TASBOT_${upper}_ASSET_NAME,")
    [void]$registry.AppendLine("    .source_path   = TASBOT_${upper}_ASSET_SOURCE_PATH,")
    [void]$registry.AppendLine("    .source_pool   = TASBOT_${upper}_ASSET_SOURCE_POOL,")
    [void]$registry.AppendLine("    .source_rule   = TASBOT_${upper}_ASSET_SOURCE_RULE,")
    [void]$registry.AppendLine("    .source_sha256 = TASBOT_${upper}_ASSET_SOURCE_SHA256,")
    [void]$registry.AppendLine("    .width         = TASBOT_${upper}_ASSET_WIDTH,")
    [void]$registry.AppendLine("    .height        = TASBOT_${upper}_ASSET_HEIGHT,")
    [void]$registry.AppendLine("    .frame_count   = TASBOT_${upper}_ASSET_FRAME_COUNT,")
    [void]$registry.AppendLine("    .frame_delays_ms = g_tasbot_${lower}_frame_delays_ms,")
    [void]$registry.AppendLine("    .frame_pixels  = g_tasbot_${lower}_pixels,")
    [void]$registry.AppendLine("};")
    [void]$registry.AppendLine("")
}

# Playlist array
[void]$registry.AppendLine("static const tasbot_embedded_animation_t* const kTasbotAnimationPlaylist[] = {")
foreach ($anim in $playlistAnimations) {
    [void]$registry.AppendLine("    &g_tasbot_$($anim.SymbolPrefix)_animation,")
}
[void]$registry.AppendLine("};")

$registryPath = Join-Path $OutputDir "animation_registry.generated.h"
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($registryPath, $registry.ToString(), $utf8NoBom)

Write-Host "Done. Registry written to: $registryPath"
