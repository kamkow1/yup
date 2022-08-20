#!/usr/bin/bash

pos_args=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -m|--mode)
            build_mode=$2
            shift
            shift
        ;;
        -*|--*)
            echo "unknown option $1"
            exit 1
            ;;
        *)
        pos_args+=("$1")
        shift
        ;;
    esac
done

set -- "${pos_args[@]}"

echo "SELECTED BUILD MODE: $build_mode"

if [[ ! -d build/ ]]; then
    mkdir build/
fi

if [[ ! -x $(which ninja) ]]; then
    echo "ninja is not installed! run sudo apt get install ninja-build to install ninja"
    echo "building with ninja is advised, since it's faster"
    echo "you can always use make. change the line no.43 in this script and delete this if check"
    exit 1
fi

cd build/

conan install ..

cmake .. -G Ninja -DCMAKE_BUILD_TYPE="$build_mode"

ninja -j 8
