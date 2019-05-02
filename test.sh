##!/bin/bash

try(){
    expected="$1"
    input="$2"

    ## build
    ./arcc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp

    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "[Failed] $input => $actual"
        echo "$expected is expected, but $actual"
        exit 1
    fi
}

try 5 '2+3;'
try 1 '12-11;'
try 43 '40 +    3;'
try 9 '3 * 3;'
try 8 '24 / 3;'
try 10 '2 * (3 + 2);'
try 3 '-5 + 8;'
try 100 '+100 + 0;'
try 0 '4 == 1;'
try 1 '1 == 1;'
try 0 '8 != 8;'
try 1 '0 == 0;'
try 1 '1 < 2;'
try 0 '100 < 5;'
try 1 '913 <= 1000;'
try 0 '1 <= 0;'
try 1 '1 > 0;'
try 0 '0 > 1;'
try 1 '1 >= 0;'
try 0 '0 >= 1;'
try 0 'a = 1;'
echo ok
