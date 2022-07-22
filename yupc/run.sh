showIR=$1
verbose=$2

cmd="./build/bin/yupc build --source demo_proj/main.yup --arch x86-64 "

if [ "$showIR" = "ir" ]; then
  cmd="${cmd} --ir"
fi

if [ "$verbose" = "v" ]; then
  cmd="${cmd} -v"
fi

eval "$cmd"
