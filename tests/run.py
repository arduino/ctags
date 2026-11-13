"""Compare ctags output against the expected output of the shipped ctags.

Usage:
  run.py --ctags PATH [--tests NAME[,NAME...]] [--verbose]
  run.py --ctags PATH --update
  run.py --list

Each case is a cases/<name>.cpp file, the input arduino-cli passed to ctags
for one of the sketches in sketches/ (see capture.sh). ctags runs on it with
arduino-cli's flags and its output must match cases/<name>.ctags byte for
byte, except for CRLF line endings, which are compared as LF. --update
writes the .ctags files instead; run it with the reference ctags only.
Exits with status 1 if any case fails.
"""

import argparse
import difflib
import subprocess
import sys
import time
from pathlib import Path

# arduino-cli's tools.ctags.pattern (internal/arduino/builder/internal/
# preprocessor/ctags.go), without the source file.
CTAGS_FLAGS = ["-u", "--language-force=c++", "-f", "-", "--c++-kinds=svpf",
               "--fields=KSTtzns", "--line-directives"]

CASES = Path(__file__).resolve().parent / "cases"

COLOR_GREEN = "\033[92m"
COLOR_RED = "\033[91m"
COLOR_RESET = "\033[0m"


def run_case(ctags, case, update):
    # Run from the cases directory with a relative path, so the output is the
    # same on every host.
    proc = subprocess.run([ctags] + CTAGS_FLAGS + [case.name], cwd=CASES,
                          capture_output=True, timeout=60)
    output = proc.stdout.replace(b"\r\n", b"\n")
    expected_path = case.with_suffix(".ctags")
    result = {"exit": proc.returncode, "stderr": proc.stderr.decode(errors="replace"),
              "output": output, "failed": []}

    if proc.returncode != 0:
        result["failed"].append("exit code %d" % proc.returncode)
    elif update:
        expected_path.write_bytes(output)
    elif not expected_path.exists():
        result["failed"].append("no %s (run with --update)" % expected_path.name)
    else:
        expected = expected_path.read_bytes()
        if output != expected:
            result["failed"].append("output differs from %s" % expected_path.name)
            result["diff"] = list(difflib.unified_diff(
                expected.decode(errors="replace").splitlines(),
                output.decode(errors="replace").splitlines(),
                "expected", "actual", lineterm=""))
    return result


def print_result(name, result, time_ms):
    s = "Test (%s)" % name
    padding = "." * max(60 - len(s), 3)
    if result["failed"]:
        status = COLOR_RED + "FAILED" + COLOR_RESET
    else:
        status = COLOR_GREEN + "PASSED" + COLOR_RESET
    print(s + padding + status + " (%dms)" % time_ms, flush=True)


def print_details(result, verbose):
    for label in result["failed"]:
        print(COLOR_RED + "  failed: %s" % label + COLOR_RESET)
    for line in result.get("diff", []):
        print("  %s" % line)
    for line in result["stderr"].splitlines():
        print("  stderr: %s" % line)
    if verbose:
        for line in result["output"].decode(errors="replace").splitlines():
            print("  | %s" % line)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ctags")
    parser.add_argument("--tests", help="comma-separated case names")
    parser.add_argument("--list", action="store_true", help="list cases")
    parser.add_argument("--update", action="store_true",
                        help="write the expected output instead of comparing")
    parser.add_argument("--verbose", action="store_true", help="print ctags' output")
    args = parser.parse_args()

    cases = sorted(CASES.glob("*.cpp"))
    if args.list:
        for case in cases:
            print(case.stem)
        return 0
    if not args.ctags:
        parser.error("--ctags is required")

    ctags = str(Path(args.ctags).resolve())
    selected = set(args.tests.split(",")) if args.tests else None
    version = subprocess.run([ctags, "--version"], capture_output=True,
                             text=True).stdout.splitlines()[0]
    print("Testing %s (%s)\n" % (ctags, version))

    passed = failed = 0
    total_start = time.monotonic()
    for case in cases:
        if selected and case.stem not in selected:
            continue
        start = time.monotonic()
        result = run_case(ctags, case, args.update)
        print_result(case.stem, result, (time.monotonic() - start) * 1000)
        if result["failed"]:
            failed += 1
        else:
            passed += 1
        if args.verbose or result["failed"]:
            print_details(result, args.verbose)

    total_ms = (time.monotonic() - total_start) * 1000
    stats = "Total: %d | Passed: %d | Failed: %d | Time: %dms" % (
        passed + failed, passed, failed, total_ms)
    separator = "=" * len(stats)
    print("\n" + separator)
    print("Total: %d | " % (passed + failed) +
          COLOR_GREEN + "Passed: %d" % passed + COLOR_RESET + " | " +
          COLOR_RED + "Failed: %d" % failed + COLOR_RESET + " | " +
          "Time: %dms" % total_ms)
    print(separator)
    if failed:
        print(COLOR_RED + "Some tests FAILED." + COLOR_RESET)
    else:
        print(COLOR_GREEN + "All tests PASSED." + COLOR_RESET)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
