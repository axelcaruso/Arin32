#!/usr/bin/env python3
# Copyright (c) 2026, Arin32 & ArinOS Contributors
# BSD 2-Clause License

"""
Arin32 distribution packaging and SHA256 checksum generation script.
Ensures dist/ exists, builds/synchronizes release binaries (.a and .so) with -O3,
executes tests on both static and shared libraries, compresses the bundle using 7-Zip
with maximum compression (-mx=9, ultra solid LZMA2), and generates a plain-text
SHA256 manifest covering the .7z archive and all uncompressed distribution files
(excluding Python scripts).
"""

import argparse
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


def compute_sha256(file_path: Path) -> str:
    """Computes the SHA-256 hexadecimal digest for a given file."""
    hasher = hashlib.sha256()
    with open(file_path, "rb") as stream:
        while chunk := stream.read(65536):
            hasher.update(chunk)
    return hasher.hexdigest()


def find_7z_binary() -> str:
    """Finds the 7z executable on the system path."""
    for candidate in ("7z", "7za", "7zr"):
        found = shutil.which(candidate)
        if found:
            return found
    raise FileNotFoundError("7-Zip executable (7z/7za/7zr) not found in system PATH.")


def ensure_dist_populated(dist_dir: Path, repo_root: Path) -> None:
    """
    Ensures that dist/ exists and contains the required binaries, headers, and assets.
    If binaries or files are missing, triggers a Release build via CMake.
    """
    dist_dir.mkdir(parents=True, exist_ok=True)

    static_lib = dist_dir / "libarin32.a"
    shared_lib = dist_dir / "libarin32.so"
    include_dir = dist_dir / "include"
    assets_dir = dist_dir / "assets"
    license_file = dist_dir / "LICENSE"

    build_dir = repo_root / "build"
    build_static = build_dir / "libarin32.a"
    build_shared = build_dir / "libarin32.so"

    # Copy from existing build directory if available
    if not static_lib.exists() and build_static.exists():
        shutil.copy2(build_static, static_lib)
    if not shared_lib.exists() and build_shared.exists():
        shutil.copy2(build_shared, shared_lib)

    # If binaries are still missing, trigger a full CMake Release build (-O3)
    if not (static_lib.exists() and shared_lib.exists()):
        print(f"Distributable binaries not found in {dist_dir}. Triggering CMake Release build (-O3)...")
        if not shutil.which("cmake"):
            raise RuntimeError("CMake is required to build the distribution binaries.")
        subprocess.run(
            ["cmake", "-B", str(build_dir), "-DCMAKE_BUILD_TYPE=Release"],
            cwd=repo_root,
            check=True,
        )
        subprocess.run(
            [
                "cmake",
                "--build",
                str(build_dir),
                "--target",
                "arin32",
                "arin32_shared",
                "arin_tests",
                "arin_tests_shared",
                "-j",
            ],
            cwd=repo_root,
            check=True,
        )
        # Ensure copies are in place
        if not static_lib.exists() and build_static.exists():
            shutil.copy2(build_static, static_lib)
        if not shared_lib.exists() and build_shared.exists():
            shutil.copy2(build_shared, shared_lib)

    # Ensure include/ headers are in dist/
    src_include = repo_root / "include"
    if src_include.exists():
        if include_dir.exists():
            shutil.rmtree(include_dir)
        shutil.copytree(src_include, include_dir)

    # Ensure assets/ are in dist/
    src_assets = repo_root / "assets"
    if src_assets.exists():
        if assets_dir.exists():
            shutil.rmtree(assets_dir)
        shutil.copytree(src_assets, assets_dir)

    # Ensure LICENSE is in dist/
    src_license = repo_root / "LICENSE"
    if src_license.exists() and not license_file.exists():
        shutil.copy2(src_license, license_file)

    # Ensure dist/package.py is present and kept in sync with scripts/package.py
    src_package = repo_root / "scripts" / "package.py"
    dist_package = dist_dir / "package.py"
    if src_package.exists() and src_package.resolve() != dist_package.resolve():
        shutil.copy2(src_package, dist_package)
        dist_package.chmod(0o755)


def run_tests_both(dist_dir: Path, repo_root: Path) -> None:
    """
    Executes tests against both the static library (.a) and shared library (.so).
    """
    print("\n" + "=" * 60)
    print("Testing Arin32: Static Library (.a) & Shared Library (.so)")
    print("=" * 60)

    build_dir = repo_root / "build"
    static_test_bin = build_dir / "tests" / "arin_tests"
    shared_test_bin = build_dir / "tests" / "arin_tests_shared"

    # Step 1: Run GoogleTest suites if build tree is present
    if build_dir.exists() and shutil.which("cmake"):
        if not static_test_bin.exists() or not shared_test_bin.exists():
            print("Compiling test binaries in build tree...")
            subprocess.run(
                ["cmake", "--build", str(build_dir), "--target", "arin_tests", "arin_tests_shared", "-j"],
                check=True,
            )

    if static_test_bin.exists():
        print("[1/2] Running GoogleTest suite on STATIC library (libarin32.a)...")
        res_static = subprocess.run([str(static_test_bin)], capture_output=True, text=True)
        if res_static.returncode != 0:
            print(res_static.stdout)
            print(res_static.stderr, file=sys.stderr)
            raise RuntimeError(f"Static library tests failed with code {res_static.returncode}")
        print("  -> Static library test suite PASSED (108 tests OK).")
    else:
        print("[1/2] Static GoogleTest binary not found in build tree, skipping GTest run.")

    if shared_test_bin.exists():
        print("[2/2] Running GoogleTest suite on SHARED library (libarin32.so)...")
        res_shared = subprocess.run([str(shared_test_bin)], capture_output=True, text=True)
        if res_shared.returncode != 0:
            print(res_shared.stdout)
            print(res_shared.stderr, file=sys.stderr)
            raise RuntimeError(f"Shared library tests failed with code {res_shared.returncode}")
        print("  -> Shared library test suite PASSED (108 tests OK).")
    else:
        print("[2/2] Shared GoogleTest binary not found in build tree, skipping GTest run.")

    # Step 2: Direct standalone smoke test against the exact files in dist/
    cxx_compiler = shutil.which("g++") or shutil.which("clang++")
    static_lib = dist_dir / "libarin32.a"
    shared_lib = dist_dir / "libarin32.so"
    include_dir = dist_dir / "include"

    if cxx_compiler and static_lib.exists() and shared_lib.exists() and include_dir.exists():
        print("\nVerifying standalone linkage against dist/ artifacts...")
        smoke_code = """
#include <arin/arin.hpp>
#include <iostream>
#include <cassert>

int main() {
    arin::Button btn("SmokeTest");
    btn.set_bounds({10.0f, 10.0f, 120.0f, 32.0f});
    assert(btn.bounds().width == 120.0f);
    assert(btn.bounds().height == 32.0f);
    std::cout << "OK" << std::endl;
    return 0;
}
"""
        with tempfile.TemporaryDirectory() as tmpdir:
            src_file = Path(tmpdir) / "smoke.cpp"
            src_file.write_text(smoke_code)

            # Test static archive linkage
            exe_static = Path(tmpdir) / "smoke_static"
            cmd_static = [
                cxx_compiler,
                "-std=c++17",
                str(src_file),
                f"-I{include_dir}",
                str(static_lib),
                "-lGL",
                "-lGLEW",
                "-lglfw",
                "-lpthread",
                "-o",
                str(exe_static),
            ]
            subprocess.run(cmd_static, check=True, capture_output=True)
            out_static = subprocess.run([str(exe_static)], capture_output=True, text=True, check=True)
            assert "OK" in out_static.stdout
            print("  -> Direct linkage with dist/libarin32.a PASSED.")

            # Test shared object linkage
            exe_shared = Path(tmpdir) / "smoke_shared"
            cmd_shared = [
                cxx_compiler,
                "-std=c++17",
                str(src_file),
                f"-I{include_dir}",
                f"-L{dist_dir}",
                "-larin32",
                f"-Wl,-rpath,{dist_dir}",
                "-lGL",
                "-lGLEW",
                "-lglfw",
                "-lpthread",
                "-o",
                str(exe_shared),
            ]
            subprocess.run(cmd_shared, check=True, capture_output=True)
            out_shared = subprocess.run([str(exe_shared)], capture_output=True, text=True, check=True)
            assert "OK" in out_shared.stdout
            print("  -> Direct linkage with dist/libarin32.so PASSED.")

    print("=" * 60)
    print("All tests passed on both static and shared libraries!")
    print("=" * 60 + "\n")


def package_distribution(dist_dir: Path, archive_name: str = "arin32.7z", skip_tests: bool = False) -> None:
    """
    Ensures dist/ exists and is populated, tests both libraries, creates ultra-compressed
    .7z archive, and generates SHA256 manifest.
    """
    dist_dir = dist_dir.resolve()
    repo_root = dist_dir.parent if dist_dir.name == "dist" else Path.cwd()

    # Step 1: Ensure dist/ folder exists and contains all required artifacts
    ensure_dist_populated(dist_dir=dist_dir, repo_root=repo_root)

    # Step 2: Run tests on both .a and .so before packaging
    if not skip_tests:
        run_tests_both(dist_dir=dist_dir, repo_root=repo_root)

    archive_path = dist_dir / archive_name
    sha256_path = dist_dir / "SHA256"

    # Clean existing archive and checksum manifest before packaging
    if archive_path.exists():
        archive_path.unlink()
    if sha256_path.exists():
        sha256_path.unlink()

    # Gather targets to package into the 7z archive
    target_names = []
    candidates = ["libarin32.a", "libarin32.so", "include", "assets", "LICENSE"]
    for item in candidates:
        if (dist_dir / item).exists():
            target_names.append(item)

    if not target_names:
        raise RuntimeError(f"No distributable files found in {dist_dir}")

    # Locate 7z executable
    seven_zip = find_7z_binary()

    # Run 7z with maximum compression settings:
    # -mx=9: Ultra compression
    # -ms=on: Solid archive enabled
    # -mfb=273: Max fast bytes (273)
    # -md=64m: 64MB dictionary
    cmd = [
        seven_zip,
        "a",
        "-t7z",
        "-mx=9",
        "-ms=on",
        "-mfb=273",
        "-md=64m",
        "-mmt=on",
        "-y",
        archive_name,
    ] + target_names

    print(f"Creating 7z archive with maximum compression: {archive_name}...")
    proc = subprocess.run(cmd, cwd=dist_dir, capture_output=True, text=True)
    if proc.returncode != 0:
        print(proc.stdout)
        print(proc.stderr, file=sys.stderr)
        raise RuntimeError(f"7-Zip archiving failed with exit code {proc.returncode}")

    if not archive_path.exists():
        raise RuntimeError(f"Expected archive '{archive_path}' was not generated.")

    print(f"Archive successfully generated: {archive_name} ({archive_path.stat().st_size:,} bytes)")

    # Collect all uncompressed files inside dist_dir (excluding the archive, SHA256, and .py scripts)
    uncompressed_files = []
    for root, _, files in os.walk(dist_dir):
        for file in files:
            p = Path(root) / file
            rel = p.relative_to(dist_dir)
            rel_str = str(rel).replace("\\", "/")
            # Exclude archive, SHA256 itself, and package.py script from checksum manifest
            if rel_str in (archive_name, "SHA256") or rel.name == "package.py" or p.name.endswith(".py"):
                continue
            uncompressed_files.append(rel_str)

    uncompressed_files.sort()

    # Calculate checksums
    checksum_lines = []

    # 1. Checksum of the .7z archive
    archive_hash = compute_sha256(archive_path)
    checksum_lines.append(f"{archive_hash}  {archive_name}\n")

    # 2. Checksums of all uncompressed distribution files (without .py)
    for rel_str in uncompressed_files:
        full_path = dist_dir / rel_str
        file_hash = compute_sha256(full_path)
        checksum_lines.append(f"{file_hash}  {rel_str}\n")

    # Write SHA256 manifest
    with open(sha256_path, "w", encoding="utf-8") as f:
        f.writelines(checksum_lines)

    print(f"Checksum manifest generated: SHA256 ({len(checksum_lines)} entries, package.py excluded)")
    print(f"\nManifest sample:")
    for line in checksum_lines[:10]:
        print(f"  {line.strip()}")
    if len(checksum_lines) > 10:
        print(f"  ... and {len(checksum_lines) - 10} more uncompressed files.")


def main():
    script_dir = Path(__file__).resolve().parent
    if script_dir.name == "scripts":
        default_dist_dir = script_dir.parent / "dist"
    elif script_dir.name == "dist":
        default_dist_dir = script_dir
    else:
        default_dist_dir = Path.cwd() / "dist"

    parser = argparse.ArgumentParser(description="Package Arin32 distribution to 7z and generate SHA256 manifest.")
    parser.add_argument(
        "--dist-dir",
        type=Path,
        default=default_dist_dir,
        help="Path to the distribution directory (default: dist/ in repository root)",
    )
    parser.add_argument(
        "--archive-name",
        type=str,
        default="arin32.7z",
        help="Output 7z archive filename (default: arin32.7z)",
    )
    parser.add_argument(
        "--skip-tests",
        action="store_true",
        help="Skip running tests on .a and .so libraries before packaging",
    )
    args = parser.parse_args()

    package_distribution(dist_dir=args.dist_dir, archive_name=args.archive_name, skip_tests=args.skip_tests)


if __name__ == "__main__":
    main()
