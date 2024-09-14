#!/usr/bin/env python3
import subprocess
from SCons.Script import Import, DefaultEnvironment

env = DefaultEnvironment()
if not ("IsCleanTarget" in dir(env) and env.IsCleanTarget()) and not env.IsIntegrationDump():
    Import("env")

    print("Running custom script before build...")
    command = ["python3", "scripts/image_to_code.py", "resources/images", "StaticImages", "lib/images"]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running script: {result.stderr}")
    else:
        print(result.stdout)
