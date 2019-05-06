##!/bin/bash
try(){
    expected="$1"
    input="$2"
    echo "\033[0;31m[test target : $input]\033[0;39m"

    ## build
    ./arcc "$input" > tmp.s
    gcc -o tmp tmp.s foo.o
    ./tmp

    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "\e[33m$input => $actual\e[m\n"
    else
        echo "[Failed] $input => $actual"
        echo "$expected is expected, but $actual"
        exit 1
    fi
}

try 5 'main(){2+3;}'
try 1 'main(){12-11;}'
try 43 'main(){40 +    3;}'
try 9 'main(){3 * 3;}'
try 8 'main(){24 / 3;}'
try 10 'main(){2 * (3 + 2);}'
try 26 'main(){1 + (2 * 3) * 4 + 1;}'
try 3 'main(){-5 + 8;}'
try 100 'main(){+100 + 0;}'
try 0 'main(){4 == 1;}'
try 1 'main(){1 == 1;}'
try 0 'main(){8 != 8;}'
try 1 'main(){0 == 0;}'
try 1 'main(){1 < 2;}'
try 0 'main(){100 < 5;}'
try 1 'main(){913 <= 1000;}'
try 0 'main(){1 <= 0;}'
try 1 'main(){1 > 0;}'
try 0 'main(){0 > 1;}'
try 1 'main(){1 >= 0;}'
try 0 'main(){0 >= 1;}'
try 1 'main(){a = 1;}'
try 123 'main(){return 123;}'
try 1 'main(){abc = 1;}'
try 2 'main(){a = 1 + 1; return a;}'
try 1 'main(){ab = 0; x = 1;}'
try 5 'main(){a = 5; b = 10; return a;}'
try 10 'main(){a = 5; b = 10; return b;}'
try 15 'main(){a = 5; b = 10; return a + b;}'
try 24 'main(){a = 2; b = 3; c = 4; return a * b * c;}'
# [NOT WORKING] big int.
# try 1000 'main(){a = 2; b = 50; c = 10; return a * b * c;}'
try 2 'main(){a = 1; if(a == 1){ return 2; }}'
try 2 'main(){a = 1; if(a == 1) return 2;}'
try 11 'main(){a = 1; if(1==a){return 11;}}'
try 10 'main(){a = 1; if(a == 1){ 1 + 1; return 10; } return 8;}'
try 8 'main(){if(1 == 1) return 8;}'
try 5 'main(){if(1 == 1){ a = 3; return 5; }}'
try 9 'main(){if(1 == 0){return 1;} else{ return 9;}}'
try 5 'main(){if(1==1){if(2==2){return 5;}}}'
try 10 'main(){if(1==1){if(1==2){return 5;}else{return 10;}}}'
try 1 'main(){return 1 == 1 && 2 == 2;}'
try 5 'main(){if(1 == 1 && 2 == 2) return 5;}'
try 1 'main(){return 1 == 1 || 1 == 2;}'
try 5 'main(){if(1 == 1 || 1 == 1) return 5;}'
try 5 'main(){if(1 == 2 || 1 == 1) return 5;}'
try 5 'main(){if(1 == 1 || 1 == 2) return 5;}'
try 10 'main(){if (1 == 2 || 2 ==1){return 5;} return 10;}'
try 99 'main(){a = 5; if(1 == 1 && a == 5){return 99;}}'
try 4 'main(){a = 3; a+=1; return a;}'
try 4 'main(){a = 5; a-=1; return a;}'
try 4 'main(){a = 2; a*=2; return a;}'
try 4 'main(){a = 8; a/=2; return a;}'
try 3 'main(){if(1==0){return 2;}else if(2 == 2){return 3;}else{return 4;}}'
try 10 'main(){ if(1==0){return 2;} else if(1 == 0){return 5;} return 10;}'
try 2 'main(){a = 1; while(a == 1){ a += 1;} return a;}'
try 5 'main(){for(a = 1; a<5; a+=1){ a+=1;} return a;}'
try 10 'main(){for(;;){return 10;}}'
try 10 'main(){for(a=10;;){return a;}}'
try 10 'main(){a = 10; for(;a<100;){return a;}}'
try 10 'main(){a = 10; for(;;a+=1){return a;}}'
try 100 'main(){a = 1; for(;a<100;){a+=1;} return a;}'
try 5 'main(){return foo();}'
try 5 'bar(){return 5;} main(){return bar();}'
try 10 'bar(){a = 5; return a;} main(){a = 10; return a;}'
try 5 'plus(x, y){return x + y;} main(){ return plus(3,2);}'
try 10 'main(){x = 0; for(i = 0; i < 10; i+=1){x+=1;} return x;}'
try 55 'sum(m, n) { acc = 0; for (i = m; i < n; i = i + 1) acc = acc + i; return acc;} main() {return sum(0, 11);}'
try 30 'rec(n){if(n == 1){return 30;} return rec(1);} main(){return rec(2);}'
try 4 'bar(){return 2;} main(){return bar() + bar();}'
try 30 'bar(n){return n;} main(){return bar(10) + bar(20);}'
try 30 'bar(n){return n;} main(){return bar(5+5) + bar(40/2);}'
try 18 'bar(n){return n;} main(){a = 5; return bar(a+a) + bar(40/a);}'
try 55 'fib(n){ if(n == 1 || n == 2) {return 1;} else {return fib(n-1) + fib(n-2);}} main(){ return fib(10); }'
try 89 'fib(n){ if(n == 1 || n == 2) {return 1;} else {return fib(n-1) + fib(n-2);}} main(){ return fib(11); }'
try 89 'fib(n){ if(n == 1 || n == 2) {return 1;} return fib(n-1) + fib(n-2);} main(){ return fib(11); }'
try 5 'main(){ i = 0; for(; i < 10; i+=1){if(i == 5) break; } return i;}'
try 5 'main(){ i = 0; while(i < 10){if(i == 5) break; i+=1;} return i;}'
echo ok
