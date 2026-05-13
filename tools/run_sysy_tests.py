#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess as proc
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BUILD_DIR = ROOT / "build"
DEFAULT_COMPILER = BUILD_DIR / "compiler.exe"
DEFAULT_OUT_DIR = ROOT / "test" / "custom" / "out_py"
DEFAULT_SYLIB = ROOT / "test" / "official" / "sylib.c"

MINGW_BINS = [
    Path(r"D:\vscode\x86_64-14.2.0-release-posix-seh-ucrt-rt_v12-rev2\mingw64\bin"),
]

BAD_ASM_PATTERNS = [
    "unknown",
    "v@",
    "v_tmp",
    "sp_ref",
    "li f",
]


def parse_args():
    parser = argparse.ArgumentParser(
        description="Build and test SysY cases with this repository's compiler."
    )
    parser.add_argument("-d", "--directory", default="test/custom")
    parser.add_argument("-t", "--test", help="Run one .sy file")
    parser.add_argument("-i", "--input", help="Input file for one test")
    parser.add_argument("-o", "--output", help="Expected .out file for one test")
    parser.add_argument("--compiler", default=str(DEFAULT_COMPILER))
    parser.add_argument("--out-dir", default=str(DEFAULT_OUT_DIR))
    parser.add_argument("--sylib", default=str(DEFAULT_SYLIB))
    parser.add_argument("--gcc", default="riscv64-linux-gnu-gcc")
    parser.add_argument("--qemu", default="qemu-riscv64-static")
    parser.add_argument("--timeout", type=float, default=5.0)
    parser.add_argument("-j", "--jobs", type=int, default=max(os.cpu_count() or 1, 1))
    parser.add_argument("-O1", action="store_true")
    parser.add_argument("-S", "--no-link", action="store_true")
    parser.add_argument("-n", "--no-execute", action="store_true")
    parser.add_argument("--build", action="store_true")
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("-v", "--verbose", action="store_true")
    return parser.parse_args()


def setup_path():
    existing = os.environ.get("PATH", "")
    prefixes = [str(path) for path in MINGW_BINS if path.exists()]
    if prefixes:
        os.environ["PATH"] = os.pathsep.join(prefixes + [existing])


def run_command(command, timeout=None, input_bytes=None):
    return proc.run(
        command,
        input=input_bytes,
        stdout=proc.PIPE,
        stderr=proc.STDOUT,
        timeout=timeout,
        shell=False,
    )


def build_project():
    result = run_command(["cmake", "--build", str(BUILD_DIR)])
    if result.returncode != 0:
        sys.stdout.write(result.stdout.decode("utf-8", errors="replace"))
        raise SystemExit(result.returncode)


def collect_cases(args):
    if args.test:
        sy_path = (ROOT / args.test).resolve() if not Path(args.test).is_absolute() else Path(args.test)
        in_path = Path(args.input).resolve() if args.input else sy_path.with_suffix(".in")
        out_path = Path(args.output).resolve() if args.output else sy_path.with_suffix(".out")
        return [(sy_path, in_path if in_path.exists() else None, out_path if out_path.exists() else None)]

    directory = (ROOT / args.directory).resolve() if not Path(args.directory).is_absolute() else Path(args.directory)
    cases = []
    for sy_path in sorted(directory.rglob("*.sy")):
        if "out" in sy_path.parts:
            continue
        out_path = sy_path.with_suffix(".out")
        if not out_path.exists():
            print(f"Warning: {sy_path.relative_to(ROOT)} missing .out file, compile-only")
            out_path = None
        in_path = sy_path.with_suffix(".in")
        cases.append((sy_path, in_path if in_path.exists() else None, out_path))
    return cases


def normalize_output(text):
    return "\n".join(line.strip() for line in text.strip().splitlines()).strip()


def scan_asm(asm_path):
    text = asm_path.read_text(encoding="utf-8", errors="replace")
    hits = []
    for line_no, line in enumerate(text.splitlines(), 1):
        lowered = line.lower()
        for pattern in BAD_ASM_PATTERNS:
            if pattern in lowered:
                hits.append(f"{line_no}: {line.strip()}")
                break
    return hits


def compile_case(args, sy_path, asm_path):
    command = [str(Path(args.compiler).resolve()), str(sy_path), "-S", "-o", str(asm_path)]
    if args.O1:
        command.append("-O1")
    return run_command(command, timeout=args.timeout)


def link_case(args, asm_path, exe_path):
    command = [
        args.gcc,
        "-static",
        str(asm_path),
        str(Path(args.sylib).resolve()),
        "-o",
        str(exe_path),
    ]
    return run_command(command, timeout=args.timeout)


def run_exe(args, exe_path, in_path):
    input_bytes = None
    if in_path and in_path.exists():
        input_bytes = in_path.read_bytes()
    return run_command([args.qemu, str(exe_path)], timeout=args.timeout, input_bytes=input_bytes)


def test_case(args, sy_path, in_path, out_path):
    rel = sy_path.relative_to(ROOT) if sy_path.is_relative_to(ROOT) else sy_path
    safe_name = "__".join(sy_path.relative_to(ROOT).with_suffix("").parts)
    out_dir = Path(args.out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    asm_path = out_dir / f"{safe_name}.s"
    log_path = out_dir / f"{safe_name}.log"

    start = time.perf_counter()
    try:
        compile_result = compile_case(args, sy_path, asm_path)
    except proc.TimeoutExpired:
        return rel, "COMPILE_TIMEOUT", f"Compiler timeout after {args.timeout:.2f}s"
    except Exception as exc:
        return rel, "COMPILE_ERROR", str(exc)

    log_path.write_bytes(compile_result.stdout)
    if compile_result.returncode != 0 or not asm_path.exists():
        msg = compile_result.stdout.decode("utf-8", errors="replace").strip()
        return rel, "COMPILE_FAIL", msg[:1200]

    bad_hits = scan_asm(asm_path)
    if bad_hits:
        return rel, "BAD_ASM", "\n".join(bad_hits[:10])

    if args.no_link or args.no_execute:
        elapsed = time.perf_counter() - start
        return rel, "COMPILE_ONLY", f"{elapsed:.3f}s"

    sylib = Path(args.sylib).resolve()
    if not sylib.exists():
        return rel, "COMPILE_ONLY", f"sylib not found: {sylib}"
    if not shutil.which(args.gcc):
        return rel, "COMPILE_ONLY", f"gcc not found: {args.gcc}"
    if not shutil.which(args.qemu):
        return rel, "COMPILE_ONLY", f"qemu not found: {args.qemu}"

    with tempfile.TemporaryDirectory() as tmp:
        exe_path = Path(tmp) / "a.out"
        try:
            link_result = link_case(args, asm_path, exe_path)
        except proc.TimeoutExpired:
            return rel, "LINK_TIMEOUT", f"Link timeout after {args.timeout:.2f}s"
        if link_result.returncode != 0 or not exe_path.exists():
            msg = link_result.stdout.decode("utf-8", errors="replace").strip()
            return rel, "LINK_FAIL", msg[:1200]

        try:
            run_result = run_exe(args, exe_path, in_path)
        except proc.TimeoutExpired:
            return rel, "RUN_TIMEOUT", f"Runtime timeout after {args.timeout:.2f}s"

    stdout = run_result.stdout.decode("utf-8", errors="replace").strip()
    actual = normalize_output(f"{stdout}\n{run_result.returncode}")

    if out_path is None:
        return rel, "RUN_NO_EXPECT", actual

    expected = normalize_output(out_path.read_text(encoding="utf-8", errors="replace"))
    if actual != expected:
        return rel, "MISMATCH", f"actual:\n{actual}\nexpected:\n{expected}"

    elapsed = time.perf_counter() - start
    return rel, "PASS", f"{elapsed:.3f}s"


def main():
    args = parse_args()
    setup_path()

    compiler = Path(args.compiler).resolve()
    if args.build or (not args.no_build and not compiler.exists()):
        build_project()

    if not compiler.exists():
        raise SystemExit(f"compiler not found: {compiler}")

    cases = collect_cases(args)
    if not cases:
        raise SystemExit("no .sy test cases found")

    print(f"Compiler: {compiler}")
    print(f"Cases: {len(cases)}")
    if args.no_execute:
        print("Mode: compile-only")
    else:
        print("Mode: compile + optional link/run when gcc, qemu, and sylib exist")

    results = []
    with ThreadPoolExecutor(max_workers=max(args.jobs, 1)) as pool:
        futures = [pool.submit(test_case, args, *case) for case in cases]
        for future in as_completed(futures):
            rel, status, detail = future.result()
            results.append((str(rel), status, detail))
            if args.verbose or status not in ("PASS", "COMPILE_ONLY"):
                print(f"[{status}] {rel}")
                if detail:
                    print(detail)

    results.sort(key=lambda item: item[0])
    passed = sum(1 for _, status, _ in results if status == "PASS")
    compile_only = sum(1 for _, status, _ in results if status == "COMPILE_ONLY")
    failed = len(results) - passed - compile_only

    print("\nTest results:")
    print(f"Total:        {len(results)}")
    print(f"Passed:       {passed}")
    print(f"Compile only: {compile_only}")
    print(f"Failed:       {failed}")

    if failed:
        print("\nFailed cases:")
        for rel, status, detail in results:
            if status in ("PASS", "COMPILE_ONLY"):
                continue
            print(f"- {rel}: {status}")
            if detail:
                lines = detail.splitlines()[:10]
                for line in lines:
                    print(f"  {line}")
        raise SystemExit(2)


if __name__ == "__main__":
    main()
