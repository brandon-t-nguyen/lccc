#!/bin/bash

# This will test assmebled files against other files
exe="bin/lccc-as"
outdir="tmp"

tests="./testing/as/patt/src/*.asm"
expect_dir="./testing/as/patt/exp"

final_result="PASS"
test_list=()
result_list=()
pass_count=0
fail_count=0

red='\033[0;31m'
green='\033[0;36m'
nc='\033[0m'

pass_str="${green}PASS${nc}";
fail_str="${red}FAIL${nc}";


mkdir -p tmp
rm -f tmp/*

for file in $tests
do
    test=${file##*/}           # extract the file name sans .asm extension
    test=${test%.asm}
    out_hex="tmp/$test.hex"
    out_bin="tmp/$test.bin"
    out_obj="tmp/$test.obj"

    cmd="./$exe $file -o $outdir/$test.obj --hex --bin --sym"
    echo $cmd
    
    # check if file is negative test case or not
    if [[ $test == pos_* ]]
    then
        # positive test
        test_list+=($test)
        ntests=$((ntests+1))
        echo "<==================|"
        echo "Running +test: $test"
        $cmd
        ret=$?
        
        if [ $ret == 0 ]
        then
            result_hex="$(diff -q -N $out_hex $expect_dir/$test.hex)"
            result_bin="$(diff -q -N $out_bin  $expect_dir/$test.bin)"
            result_obj="$(diff -q <(xxd $out_obj) <(xxd $expect_dir/$test.obj))"
        fi

        if [ $ret != 0 ]
        then
            # failure
            result_list+=($fail_str)
            fail_count=$((fail_count+1))
            echo -e "$test: $fail_str"
        elif [ -z "$result_hex"  ] && [ -z "$result_bin" ] && [ -z "$result_obj" ]
        then
            # success
            result_list+=($pass_str)
            pass_count=$((pass_count+1))
            echo -e "$test: $pass_str"
        else
            # failure
            result_list+=($fail_str)
            fail_count=$((fail_count+1))
            echo -e "$test: $fail_str"
            
            # print a diff: use <(nl ...) to recognize line numbers: -t to expandtab
            if [[ ! -z "$result_hex" ]]
            then
                echo "Differences in .hex file..."
                diff -t -N -y  <(nl $out_hex) <(nl $expect_dir/$test.hex)
            fi

            if [[ ! -z "$result_bin" ]]
            then
                echo "Differences in .bin file..."
                diff -t -N -y  <(nl $out_bin) <(nl $expect_dir/$test.bin)
            fi

            if [[ ! -z "$result_obj" ]]
            then
                echo "Differences in .obj file..."
                diff -t -N -y  <(xxd $out_obj) <(xxd $expect_dir/$test.obj)
            fi
        fi
        echo "|==================>"
        echo ""
        #rm -f $outdir/*
    elif [[ $test == neg_* ]]
    then
        # negative test
        test_list+=($test)
        ntests=$((ntests+1))
        echo "<==================|"
        echo "Running -test: $test"
        $cmd
        ret=$?
        if [ $ret != 0 ] && [ ! -e "$result_hex" ] && [ ! -e "$result_bin" ] && [ ! -e "$result_obj" ]
        then
            # success
            echo -e "$test: $pass_str"
            result_list+=("$pass_str")
            pass_count=$((pass_count+1))
        else
            # failure
            result_list+=("$fail_str")
            fail_count=$((fail_count+1))
            echo -e "$test: $fail_str"

            if [ -e "$result_hex" ]; then
                echo -e "Should not have produced .hex file"
            fi

            if [ -e "$result_bin" ]; then
                echo -e "Should not have produced .bin file"
            fi

            if [ -e "$result_obj" ]; then
                echo -e "Should not have produced .obj file"
            fi
        fi
        echo "|==================>"
        echo ""
        #rm -f $outdir/*
    else
        # do nothing
        echo "$file is not a test case"
    fi
done

tLen=$((pass_count+fail_count))
for (( i=0; i<tLen; i++ ))
do
    echo -e "${result_list[$i]}: ${test_list[$i]}"
done
if [ $fail_count -gt 0 ]
then
    echo -e "<========== Testing result: $fail_str ==========>"
else
    echo -e "<========== Testing result: $pass_str ==========>"
fi
echo "Passed $pass_count out of $tLen tests"
