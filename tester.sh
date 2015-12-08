#!/bin/bash

bin="./minic"           # The application (from command arg)
diff="diff -iad"   # Diff command, or what ever

run_test() {
    # Validate infile exists (do the same for out validate file)
    if [ ! -f "$1" ]; then
        printf "In file %s is missing\n" "$1"
        continue;
    fi
    if [ ! -f "$2" ]; then
        printf "Validation file %s is missing\n" "$2"
        continue;
    fi

    printf "  - test %s ... " "$1"

    # Run application, redirect in file to app, and output to out file
    "./$bin" "$1" > /dev/null

    # Execute diff
    $diff "$3" "$2"


    # Check exit code from previous command (ie diff)
    # We need to add this to a variable else we can't print it
    # as it will be changed by the if [
    # Iff not 0 then the files differ (at least with diff)
    e_code=$?
    if [ $e_code != 0 ]; then
            printf "FAIL.. : %d\n" "$e_code"
    else
            printf "OK!\n"
    fi
}

echo "[Run Test AST...]"
for file in tests/*.test.ast; do
    # Padd file_base with suffixes
    file_in="${file%%.*}.mc"             # The in file
    file_out_val="${file%%.*}.test.ast"       # The out file to check against
    file_out_tst="${file%%.*}.ast"   # The outfile from test application

    run_test $file_in $file_out_val $file_out_tst

done

echo ""
echo "[Run Test UCO...]"
for file in tests/*.test.uco; do
    # Padd file_base with suffixes
    file_in="${file%%.*}.mc"             # The in file
    file_out_val="${file%%.*}.test.uco"       # The out file to check against
    file_out_tst="${file%%.*}.uco"   # The outfile from test application

    run_test $file_in $file_out_val $file_out_tst

done

# Clean exit with status 0
exit 0