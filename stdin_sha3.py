#!/usr/bin/python

from sys import stdin
import subprocess

subprocess.run(["make"])
print("Press CTRL+D (Linux) or CTRL+Z (Windows) to exit")
print("Enter message: ", end="", flush=True)
for line in stdin:
    subprocess.run(["./sha3", line[:-1]])
    print("Enter message: ", end="", flush=True)
