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

$stream = New-Object System.Collections.Generic.List[byte]
$stream.AddRange((New-Packet -Type 0x01 -Payload ([byte[]]@(0xE1,0x09))))    # 25.29 C
$stream.AddRange((New-Packet -Type 0x02 -Payload ([byte[]]@(0x88,0x13))))    # 5.000 V
$stream.AddRange((New-Packet -Type 0x03 -Payload ([byte[]]@(0x05))))         # status flags
$stream.AddRange((New-Packet -Type 0x01 -Payload ([byte[]]@(0x10,0x27)) -CorruptChecksum))
$stream.AddRange([byte[]]@(0x00,0x12,0x34,0xAA))                              # noise + truncated start

[System.IO.File]::WriteAllBytes("data/sample_stream.bin", $stream.ToArray())
Write-Host "Wrote data/sample_stream.bin with $($stream.Count) bytes"
