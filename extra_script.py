#!/usr/bin/env python3
import os
import subprocess
from SCons.Script import Import

Import("env")

def before_build(source, target, env):
    print("Running custom script before build...")
    command = ["python3", "scripts/image_to_code.py", "resources/images", "StaticImages", "lib/images"]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running script: {result.stderr}")
        env.Exit(1)
    else:
        print(result.stdout)

env.AddPreAction("$BUILD_DIR/src/main.cpp.o", before_build)