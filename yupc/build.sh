if [[ -d build ]]; then
  rm -r build
fi

if [[ ! -d build ]]; then
  mkdir build
fi

cd build/ &&
conan install .. &&
cmake .. &&
make