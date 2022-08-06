cmd="./build/bin/yupc build --source demo_proj/main.yup"

while getopts ":r:v:" arg; do
  case $arg in
    r) showIR=$OPTARG;;
    v) verbose=$OPTARG;;
  esac
done

if [ "$showIR" = true ]; then
  cmd="${cmd} --ir"
fi

if [ "$verbose" = true ]; then
  cmd="${cmd} -v"
fi

echo $cmd

eval "$cmd"
