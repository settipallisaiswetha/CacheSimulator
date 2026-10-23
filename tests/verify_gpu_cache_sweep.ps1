
param(
    [Parameter(Mandatory = $true)]
    [string]$CsvPath
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $CsvPath)) {
    throw "CSV file not found: $CsvPath"
}

$rows = @(Import-Csv $CsvPath)

if ($rows.Count -ne 16) {
    throw "Expected 16 experiments, found $($rows.Count)"
}

$expected = @{
    "4_Baseline"  = @(0, 32, 32, 2003)
    "4_Config_A"  = @(32, 0, 32, 1683)
    "4_Config_B"  = @(32, 0, 32, 1683)
    "4_Config_C"  = @(32, 0, 32, 1683)

    "8_Baseline"  = @(0, 0, 128, 3207)
    "8_Config_A"  = @(0, 64, 64, 2007)
    "8_Config_B"  = @(64, 0, 64, 1687)
    "8_Config_C"  = @(64, 0, 64, 1687)

    "16_Baseline" = @(0, 0, 256, 3215)
    "16_Config_A" = @(0, 0, 256, 3215)
    "16_Config_B" = @(0, 128, 128, 2015)
    "16_Config_C" = @(128, 0, 128, 1695)

    "32_Baseline" = @(0, 0, 512, 3231)
    "32_Config_A" = @(0, 0, 512, 3231)
    "32_Config_B" = @(0, 0, 512, 3231)
    "32_Config_C" = @(0, 0, 512, 3231)
}

$seen = @{}

foreach ($row in $rows) {
    $key = "$($row.Warps)_$($row.Configuration)"

    if (-not $expected.ContainsKey($key)) {
        throw "Unexpected experiment: $key"
    }

    if ($seen.ContainsKey($key)) {
        throw "Duplicate experiment: $key"
    }

    $seen[$key] = $true

    $actual = @(
        [int]$row.L1Hits,
        [int]$row.L2Hits,
        [int]$row.DRAMAccesses,
        [int]$row.CompletionCycles
    )

    $wanted = $expected[$key]

    for ($i = 0; $i -lt 4; $i++) {
        if ($actual[$i] -ne $wanted[$i]) {
            throw "Mismatch in $key at metric $i. Expected $($wanted[$i]), got $($actual[$i])"
        }
    }

    $expectedLines = [int]$row.Warps * 8 * 2

    if (($actual[0] + $actual[1] + $actual[2]) -ne $expectedLines) {
        throw "Invalid cache access total for $key"
    }
}

Write-Output "All 16 GPU cache sweep regression cases passed!"
