debug=$1

if [[ -d build ]]; then
  rm -r build
fi

if [[ ! -d build ]]; then
  mkdir build
fi

if [ "$debug" = "debug" ]; then
  cd build/ &&
  conan install .. &&
  cmake .. -DCMAKE_BUILD_TYPE=Debug &&
  make
else
  cd build/ &&
  conan install .. &&
  cmake .. &&
  make
fi