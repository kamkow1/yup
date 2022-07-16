debug=$1

if [[ ! -d build ]]; then
  mkdir build
fi

if [ "$debug" = "debug" ]; then
  if [[ ! -d debug ]]; then
    mkdir debug
  fi
  cd debug/ &&
  conan install .. &&
  cmake .. -DCMAKE_BUILD_TYPE=Debug &&
  make
else
  cd build/ &&
  conan install .. &&
  cmake .. &&
  make
fi