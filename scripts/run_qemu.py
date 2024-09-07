#!/usr/bin/env python3
import os
import argparse
import tempfile

def run_tests(file):
    # prepare a temporary folder
    folder = tempfile.mkdtemp()
    
    # Copy the file to the temporary folder
    file_name = os.path.basename(file)
    file_folder = os.path.dirname(file)
    os.system(f"cp {file} {folder}/{file_name}")

    # Copy bootloader and partition
    os.system(f"mkdir -p {folder}/bootloader")
    os.system(f"mkdir -p {folder}/partition_table")
    os.system(f"cp {file_folder}/bootloader.bin {folder}/bootloader/bootloader.bin")
    os.system(f"cp {file_folder}/partitions.bin {folder}/partition_table/partition-table.bin")

    # Kill any running Docker container with the same image
    os.system("docker kill $(docker ps -q --filter ancestor=mluis/qemu-esp32)")

    # Run the Docker container
    command = f"docker run -it --rm -p 8555:80 -p 5555:5555 -v {folder}:/root/compiled mluis/qemu-esp32 \
        sh -c './flash.sh /root/compiled/{file_name} && qemu-system-xtensa -nographic -M esp32 -m 4M \
        -drive file=flash.bin,if=mtd,format=raw \
        -nic user,model=open_eth,hostfwd=tcp::80-:80'"
    os.system(command)

if __name__ == "__main__":
    # Create argument parser
    parser = argparse.ArgumentParser(description="Run tests using Docker container")

    # Add file argument
    parser.add_argument("file", help="Path to the file")

    # Parse the arguments
    args = parser.parse_args()

    # Run the tests
    run_tests(args.file)
