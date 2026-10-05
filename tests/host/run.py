#!/usr/bin/env python3
"""Run host-only protocol/lifecycle tests; requires a C++ compiler."""
from pathlib import Path
import subprocess, tempfile, os
here = Path(__file__).resolve().parent
with tempfile.TemporaryDirectory() as directory:
    binary = str(Path(directory) / "canbridge-tests")
    subprocess.run([os.environ.get("CXX", "c++"), "-std=c++11", "-Wall", "-Wextra", "-pedantic", "-fsanitize=address,undefined", "-I" + str(here), "-I" + str(here.parents[1] / "src"), str(here / "tests.cpp"), "-o", binary], check=True)
    subprocess.run([binary], check=True)
print("Protocol, diagnostics, cleanup retry and serial/CAN progress tests passed.")
