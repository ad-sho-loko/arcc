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
        echo "$expected is expected, but $actual"
        exit 1
    fi
}

try 5 '2+3'
try 1 '12-11'
try 43 '40 +    3'
try 9 '3 * 3'
try 8 '24 / 3'
echo ok
