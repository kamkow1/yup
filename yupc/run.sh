showIR=$1

if [ "$showIR" == "ir" ]; then
  ./build/bin/yupc build --source demo_proj/main.yup --arch x86-64 --ir
else
  ./build/bin/yupc build --source demo_proj/main.yup --arch x86-64
fi