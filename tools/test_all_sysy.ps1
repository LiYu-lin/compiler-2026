param(
    [string]$TestDir = "test/custom",
    [switch]$Build,
    [string]$OutDir = "test/custom/out",
    [string]$Compiler = ""
)

$ErrorActionPreference = "Continue"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir

function Resolve-InRoot([string]$Path) {
    if ([System.IO.Path]::IsPathRooted($Path)) {
        return $Path
    }
    return Join-Path $Root $Path
}

$TestDirPath = Resolve-InRoot $TestDir
$OutDirPath = Resolve-InRoot $OutDir

if (!(Test-Path $TestDirPath)) {
    throw "test directory not found: $TestDirPath"
}

if (!(Test-Path $OutDirPath)) {
    New-Item -ItemType Directory -Force -Path $OutDirPath | Out-Null
}

if ($Build) {
    Write-Host "[test_all_sysy] Building compiler..."
    & cmake --build (Join-Path $Root "build")
    if ($LASTEXITCODE -ne 0) {
        throw "cmake build failed with exit code $LASTEXITCODE"
    }
}

if ($Compiler -eq "") {
    $CompilerPath = Join-Path $Root "build/compiler.exe"
} else {
    $CompilerPath = Resolve-InRoot $Compiler
}

if (!(Test-Path $CompilerPath)) {
    throw "compiler executable not found: $CompilerPath"
}

$MingwBins = @(
    "D:\vscode\x86_64-14.2.0-release-posix-seh-ucrt-rt_v12-rev2\mingw64\bin"
)

foreach ($Bin in $MingwBins) {
    if (Test-Path $Bin) {
        $env:PATH = "$Bin;$env:PATH"
    }
}

$BadPatterns = @(
    "\bunknown\b",
    "v@",
    "v_tmp",
    "sp_ref",
    "\bli\s+f"
)

$Sources = Get-ChildItem -Path $TestDirPath -Recurse -File -Filter *.sy |
    Where-Object { $_.FullName -notlike "*\out\*" } |
    Sort-Object FullName

if ($Sources.Count -eq 0) {
    throw "no .sy files found under: $TestDirPath"
}

$Rows = @()
$FailCount = 0

foreach ($Src in $Sources) {
    $Base = $TestDirPath.TrimEnd('\', '/')
    if ($Src.FullName.StartsWith($Base)) {
        $Rel = $Src.FullName.Substring($Base.Length).TrimStart('\', '/')
    } else {
        $Rel = $Src.Name
    }
    if ($Rel.EndsWith(".sy")) {
        $Stem = $Rel.Substring(0, $Rel.Length - 3)
    } else {
        $Stem = [System.IO.Path]::GetFileNameWithoutExtension($Rel)
    }
    $FlatName = ($Stem -replace "[\\/]", "__")

    $IrPath = Join-Path $OutDirPath "$FlatName.ir"
    $AsmPath = Join-Path $OutDirPath "$FlatName.s"
    $IrLogPath = Join-Path $OutDirPath "$FlatName.emit-ir.log"
    $AsmLogPath = Join-Path $OutDirPath "$FlatName.emit-asm.log"
    $InPath = [System.IO.Path]::ChangeExtension($Src.FullName, ".in")
    $ExpectedPath = [System.IO.Path]::ChangeExtension($Src.FullName, ".out")

    Write-Host "[test_all_sysy] Testing $Rel"

    $IrOutput = & $CompilerPath $Src.FullName --emit-ir -o $IrPath 2>&1
    $IrExit = $LASTEXITCODE
    $IrOutput | Set-Content -Encoding UTF8 $IrLogPath
    $IrOk = (($IrExit -eq 0) -or (Test-Path $IrPath))

    $AsmOk = $false
    $BadHitText = ""
    if ($IrOk) {
        $AsmOutput = & $CompilerPath $Src.FullName -S -o $AsmPath 2>&1
        $AsmExit = $LASTEXITCODE
        $AsmOutput | Set-Content -Encoding UTF8 $AsmLogPath
        $AsmOk = (($AsmExit -eq 0) -or (Test-Path $AsmPath))

        if ($AsmOk) {
            $BadHits = Select-String -Path $AsmPath -Pattern $BadPatterns
            if ($BadHits) {
                $BadHitText = ($BadHits | ForEach-Object { "$($_.LineNumber):$($_.Line.Trim())" }) -join "; "
            }
        }
    } else {
        $AsmExit = $null
    }

    $Status = "PASS"
    if (!$IrOk) {
        $Status = "IR_FAIL"
    } elseif (!$AsmOk) {
        $Status = "ASM_FAIL"
    } elseif ($BadHitText -ne "") {
        $Status = "BAD_ASM"
    }

    if ($Status -ne "PASS") {
        $FailCount += 1
    }

    $Rows += [PSCustomObject]@{
        Source = $Rel
        Status = $Status
        HasInput = Test-Path $InPath
        HasExpectedOutput = Test-Path $ExpectedPath
        IR = $IrPath
        ASM = $AsmPath
        IRLog = $IrLogPath
        ASMLog = $AsmLogPath
        SuspiciousASM = $BadHitText
    }
}

$ReportPath = Join-Path $OutDirPath "batch_report.csv"
$Rows | Export-Csv -NoTypeInformation -Encoding UTF8 $ReportPath

Write-Host ""
Write-Host "[test_all_sysy] Summary"
$Rows | Format-Table Source, Status, HasInput, HasExpectedOutput -AutoSize
Write-Host "[test_all_sysy] Report: $ReportPath"

if ($FailCount -ne 0) {
    Write-Host "[test_all_sysy] Failed or suspicious cases: $FailCount"
    exit 2
}

Write-Host "[test_all_sysy] OK: all sources emitted IR and assembly without suspicious patterns."
Write-Host "[test_all_sysy] Note: .out files are expected runtime output; this repo currently has no RISC-V runner/runtime script to execute and compare them."
