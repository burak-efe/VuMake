import os
import subprocess
import sys
from pathlib import Path

# Determine platform-specific compiler command
compiler = "../../bin/windows-x86_64/slang/slangc.exe" if os.name == "nt" else "../../bin/linux-x86_64/slang/slangc"

# Collect all .slang files
slang_files = list(Path(".").rglob("*.slang"))

if not slang_files:
    print("No .slang files found.")
    sys.exit(0)

# Compile each file
for file in slang_files:

    out_file = file.with_suffix(".spv")
    print(f"Compiling: {file} → {out_file}")

    result = subprocess.run(
        [compiler, str(file),"-target", "spirv", "-fvk-use-scalar-layout","-g", "-o", out_file],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    if result.returncode != 0:
        print(f"❌ Failed to compile: {file}")
        print(result.stderr.strip())
    else:
        print(f"✅ Success: {file}")

