$ErrorActionPreference = 'Stop'

function Remove-LegacyIniSettings {
    param([Parameter(Mandatory = $true)][string] $Path)

    if (-not [System.IO.File]::Exists($Path)) {
        return
    }

    $bytes = [System.IO.File]::ReadAllBytes($Path)
    $hasUtf8Bom =
        $bytes.Length -ge 3 -and
        $bytes[0] -eq 0xEF -and
        $bytes[1] -eq 0xBB -and
        $bytes[2] -eq 0xBF
    $encoding = [System.Text.UTF8Encoding]::new($hasUtf8Bom)
    $text = [System.IO.File]::ReadAllText($Path, $encoding)
    $newline = if ($text.Contains("`r`n")) { "`r`n" } else { "`n" }
    $lines = [System.Text.RegularExpressions.Regex]::Split(
        $text,
        "\r\n|\n|\r"
    )
    $result = [System.Collections.Generic.List[string]]::new()
    $section = ''
    $changed = $false

    foreach ($line in $lines) {
        $sectionMatch = [System.Text.RegularExpressions.Regex]::Match(
            $line,
            '^\s*\[([^\]]+)\]\s*$'
        )
        if ($sectionMatch.Success) {
            $section = $sectionMatch.Groups[1].Value
            if ($section -ieq 'AccessibleOBSStudio') {
                $changed = $true
                continue
            }
        }

        if ($section -ieq 'AccessibleOBSStudio') {
            $changed = $true
            continue
        }

        if (
            $section -ieq 'Hotkeys' -and
            $line -match '^\s*accessible_obs_studio\.[^=]*='
        ) {
            $changed = $true
            continue
        }

        $result.Add($line)
    }

    if (-not $changed) {
        return
    }

    $updatedText = [string]::Join($newline, $result)
    $temporaryPath = $Path + '.accessible-studio-cleanup.tmp'
    [System.IO.File]::WriteAllText($temporaryPath, $updatedText, $encoding)
    Move-Item -LiteralPath $temporaryPath -Destination $Path -Force
}

$roamingData = [Environment]::GetFolderPath(
    [Environment+SpecialFolder]::ApplicationData
)
$localData = [Environment]::GetFolderPath(
    [Environment+SpecialFolder]::LocalApplicationData
)
$obsConfigRoot = [System.IO.Path]::Combine($roamingData, 'obs-studio')

Remove-LegacyIniSettings (
    [System.IO.Path]::Combine($obsConfigRoot, 'global.ini')
)

$profilesRoot = [System.IO.Path]::Combine($obsConfigRoot, 'basic', 'profiles')
if ([System.IO.Directory]::Exists($profilesRoot)) {
    Get-ChildItem -LiteralPath $profilesRoot -Directory | ForEach-Object {
        Remove-LegacyIniSettings (
            [System.IO.Path]::Combine($_.FullName, 'basic.ini')
        )
    }
}

$legacyLocalData = [System.IO.Path]::Combine(
    $localData,
    'AccessibleOBSStudio'
)
if ([System.IO.Directory]::Exists($legacyLocalData)) {
    Remove-Item -LiteralPath $legacyLocalData -Recurse -Force
}

$cmdKey = [System.IO.Path]::Combine(
    [Environment]::GetFolderPath([Environment+SpecialFolder]::System),
    'cmdkey.exe'
)
if ([System.IO.File]::Exists($cmdKey)) {
    & $cmdKey '/delete:AccessibleOBSStudio/OpenAI' 2>&1 | Out-Null
}

exit 0
