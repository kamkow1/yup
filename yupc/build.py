import argparse
import os
import sys
from shutil import which
import subprocess

parser = argparse.ArgumentParser(
    description="build script for the yup compiler project")

parser.add_help = True

parser.add_argument("-m", "--mode", 
    required=True,
    help="sets build mode ~ relase | debug")

args = parser.parse_args()

build_mode = args.mode

script_dir = os.path.dirname(
    os.path.realpath(__file__))

build_dir = os.path.join(script_dir, "build")

if (not os.path.isdir("build")):
    print("build directory not found!")

    os.mkdir(build_dir)

    print(f"created build directory: {build_dir}")
else:
    print("build directory already exists!")

print(f"selected build mode: {build_mode}")

os.chdir(build_dir) # set working directory to ./build

# install deps
conan_cmd = "conan install .."
proc = subprocess.Popen(conan_cmd, 
    shell=True, stdout=subprocess.PIPE)

for line in proc.stdout:
    text = line.decode()
    print(text)

proc.wait()
conan_res = proc.returncode
del proc

if (conan_res != 0):
    print(f"failed to install conan dependencies ({conan_cmd}). error code: {conan_res}")
    sys.exit(1)
else:
    print("successfully installed conan dependencies")

# generate build files
cmake_cmd = "cmake .."
cmake_build_type = f" -DCMAKE_BUILD_TYPE={build_mode} -GNinja"

cmake_cmd += cmake_build_type
proc = subprocess.Popen(cmake_cmd, 
    shell=True, stdout=subprocess.PIPE)

for line in proc.stdout:
    text = line.decode()
    print(text)

proc.wait()
cmake_res = proc.returncode
del proc

if (cmake_res != 0):
    print(f"failed to generate build files ({cmake_cmd}). error code: {cmake_res}")
    sys.exit(1)
else:
    print("successfully generated build files")

# build with ninja

is_ninja_available = which("ninja")

if (is_ninja_available is None):
    print("ninja is not installed on your system!")
    print("install ninja with sudo apt-get install ninja-build")
    sys.exit(1)

proc = subprocess.Popen("ninja", 
    shell=True, stdout=subprocess.PIPE)

for line in proc.stdout:
    text = line.decode()
    print(text)

proc.wait()
ninja_res = proc.returncode
del proc

if (ninja_res != 0):
    print("building with ninja failed!")
    sys.exit(1)

print("finished building successfully!")
