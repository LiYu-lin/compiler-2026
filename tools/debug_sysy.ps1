param(
    [string]$Source = "test/custom/manual.sy",
    [switch]$Build,
    [string]$OutDir = "test/custom/out",
    [string]$Compiler = ""
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir

function Resolve-InRoot([string]$Path) {
    if ([System.IO.Path]::IsPathRooted($Path)) {
        return $Path
    }
    return Join-Path $Root $Path
}

$InputPath = Resolve-InRoot $Source
$OutDirPath = Resolve-InRoot $OutDir

if (!(Test-Path $OutDirPath)) {
    New-Item -ItemType Directory -Force -Path $OutDirPath | Out-Null
}

if (!(Test-Path $InputPath)) {
    $InputParent = Split-Path -Parent $InputPath
    if (!(Test-Path $InputParent)) {
        New-Item -ItemType Directory -Force -Path $InputParent | Out-Null
    }

    @"
int add(int a, int b) {
    return a + b;
}

int main() {
    int x = add(40, 2);
    return x;
}
"@ | Set-Content -Encoding ASCII $InputPath
    Write-Host "[debug_sysy] Created sample SysY source: $InputPath"
}

if ($Build) {
    Write-Host "[debug_sysy] Building compiler..."
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

$BaseName = [System.IO.Path]::GetFileNameWithoutExtension($InputPath)
$IrPath = Join-Path $OutDirPath "$BaseName.ir"
$AsmPath = Join-Path $OutDirPath "$BaseName.s"
$IrLogPath = Join-Path $OutDirPath "$BaseName.emit-ir.log"
$AsmLogPath = Join-Path $OutDirPath "$BaseName.emit-asm.log"

Write-Host "[debug_sysy] Source : $InputPath"
Write-Host "[debug_sysy] Compiler: $CompilerPath"

Write-Host "[debug_sysy] Emitting IR..."
$IrOutput = & $CompilerPath $InputPath --emit-ir -o $IrPath 2>&1
$IrExit = $LASTEXITCODE
$IrOutput | Set-Content -Encoding UTF8 $IrLogPath
if (($null -eq $IrExit) -and (Test-Path $IrPath)) {
    $IrExit = 0
}

if (($IrExit -ne 0) -and !(Test-Path $IrPath)) {
    Write-Host "[debug_sysy] IR failed. Log: $IrLogPath"
    $IrOutput
    exit $IrExit
} elseif ($IrExit -ne 0) {
    Write-Host "[debug_sysy] IR command returned $IrExit, but output file exists; continuing."
}

Write-Host "[debug_sysy] Emitting assembly..."
$AsmOutput = & $CompilerPath $InputPath -S -o $AsmPath 2>&1
$AsmExit = $LASTEXITCODE
$AsmOutput | Set-Content -Encoding UTF8 $AsmLogPath
if (($null -eq $AsmExit) -and (Test-Path $AsmPath)) {
    $AsmExit = 0
}

if (($AsmExit -ne 0) -and !(Test-Path $AsmPath)) {
    Write-Host "[debug_sysy] ASM failed. Log: $AsmLogPath"
    $AsmOutput
    exit $AsmExit
} elseif ($AsmExit -ne 0) {
    Write-Host "[debug_sysy] ASM command returned $AsmExit, but output file exists; continuing."
}

Write-Host "[debug_sysy] IR : $IrPath"
Write-Host "[debug_sysy] ASM: $AsmPath"

$BadPatterns = @(
    "\bunknown\b",
    "v@",
    "v_tmp",
    "sp_ref",
    "\bli\s+f"
)

$BadHits = Select-String -Path $AsmPath -Pattern $BadPatterns
if ($BadHits) {
    Write-Host "[debug_sysy] Suspicious assembly patterns found:"
    $BadHits | ForEach-Object {
        Write-Host ("  {0}:{1}: {2}" -f $_.Path, $_.LineNumber, $_.Line.Trim())
    }
    exit 2
}

Write-Host "[debug_sysy] OK: compile, IR emit, ASM emit, and basic assembly scan passed."
