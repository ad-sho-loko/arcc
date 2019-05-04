##!/bin/bash

try(){
    expected="$1"
    input="$2"
    echo "\033[0;31m[test target : $input]\033[0;39m"

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
try 1 'a = 1;'
try 123 'return 123;'
try 1 'abc = 1;'
try 2 'a = 1 + 1; return a;'
try 1 'ab = 0; x = 1;'
try 2 'a = 1; if(a == 1){ return 2; }'
try 2 'a = 1; if(a == 1) return 2;'
try 11 'a = 1; if(1==a){return 11;}'
try 10 'a = 1; if(a == 1){ 1 + 1; return 10; } return 8;'
try 8 'if(1 == 1) return 8;'
try 5 'if(1 == 1){ a = 3; return 5; }'
try 9 'if(1 == 0){return 1;} else{ return 9;}'
try 5 'if(1==1){if(2==2){return 5;}}'
try 10 'if(1==1){if(1==2){return 5;}else{return 10;}}'
try 1 'return 1 == 1 && 2 == 2;'
try 5 'if(1 == 1 && 2 == 2) return 5;'
try 1 'return 1 == 1 || 1 == 2;'
try 5 'if(1 == 1 || 1 == 1) return 5;'
try 5 'if(1 == 2 || 1 == 1) return 5;'
try 5 'if(1 == 1 || 1 == 2) return 5;'
try 99 'a = 5; if(1 == 1 && a == 5){return 99;}'
try 4 'a = 3; a+=1; return a;'
try 4 'a = 5; a-=1; return a;'
try 4 'a = 2; a*=2; return a;'
try 4 'a = 8; a/=2; return a;'
try 3 'if(1==0){return 2;}else if(2 == 2){return 3;}else{return 4;}'
try 10 'if(1==0){return 2;}else if(1 == 0){return 5;}return 10;'
echo ok
