
file="$1"
make
./minic $file
./ucodei ${file%%.*}.uco