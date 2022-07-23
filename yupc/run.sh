cmd="./build/bin/yupc build --source demo_proj/main.yup"

while getopts ":ir:v:o:" arg; do
  case $arg in
    ir) showIR=$OPTARG;;
    v) verbose=$OPTARG;;
    o) obj=$OPTARG;;
  esac
done

if [ "$showIR" = true ]; then
  cmd="${cmd} --ir"
fi

if [ "$verbose" = true ]; then
  cmd="${cmd} -v"
fi

if [ "$obj" = true ]; then
  cmd="${cmd} -o"
fi

echo $cmd

eval "$cmd"
