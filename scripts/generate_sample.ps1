$ErrorActionPreference = "Stop"

if (-not (Test-Path "data")) {
    New-Item -ItemType Directory -Path "data" | Out-Null
}

function Get-Checksum {
    param([byte[]] $FrameNoChecksum)

    $sum = 0
    foreach ($b in $FrameNoChecksum) {
        $sum = ($sum + $b) -band 0xFF
    }
    return [byte]$sum
}

function New-Packet {
    param(
        [byte] $Type,
        [byte[]] $Payload,
        [switch] $CorruptChecksum
    )

    $header = [byte[]]@(0xAA, $Type, [byte]$Payload.Length)
    $frame = New-Object System.Collections.Generic.List[byte]
    $frame.AddRange($header)
    $frame.AddRange($Payload)

    $checksum = Get-Checksum -FrameNoChecksum $frame.ToArray()
    if ($CorruptChecksum) {
        $checksum = [byte]($checksum -bxor 0xFF)
    }

    $frame.Add($checksum)
    return $frame.ToArray()
}

function Add-Bytes {
    param(
        [System.Collections.Generic.List[byte]] $Target,
        [byte[]] $Bytes
    )

    foreach ($value in $Bytes) {
        $Target.Add($value)
    }
}

$stream = New-Object System.Collections.Generic.List[byte]
Add-Bytes -Target $stream -Bytes (New-Packet -Type 0x01 -Payload ([byte[]]@(0xE1,0x09)))    # 25.29 C
Add-Bytes -Target $stream -Bytes (New-Packet -Type 0x02 -Payload ([byte[]]@(0x88,0x13)))    # 5.000 V
Add-Bytes -Target $stream -Bytes (New-Packet -Type 0x03 -Payload ([byte[]]@(0x05)))         # status flags
Add-Bytes -Target $stream -Bytes (New-Packet -Type 0x01 -Payload ([byte[]]@(0x10,0x27)) -CorruptChecksum)
Add-Bytes -Target $stream -Bytes ([byte[]]@(0x00,0x12,0x34,0xAA))                              # noise + truncated start

[System.IO.File]::WriteAllBytes("data/sample_stream.bin", $stream.ToArray())
($stream | ForEach-Object { $_.ToString("X2") }) -join " " | Set-Content -Path "data/sample_stream.hex" -NoNewline
Write-Host "Wrote data/sample_stream.bin with $($stream.Count) bytes"
Write-Host "Wrote data/sample_stream.hex with $($stream.Count) byte values"
