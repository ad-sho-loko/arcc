##!/bin/bash
try(){
    expected="$1"
    input="$2"
    echo "\033[0;31m[test target : $input]\033[0;39m"

    ## build
    ./arcc "$input" > tmp.s
    gcc -o tmp tmp.s ./test/foo.o
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

# prepare
gcc -c ./test/foo.c -o ./test/foo.o

# act
try 5 'int main(){2+3;}'
try 1 'int main(){12-11;}'
try 43 'int main(){40 +    3;}'
try 9 'int main(){3 * 3;}'
try 8 'int main(){24 / 3;}'
try 10 'int main(){2 * (3 + 2);}'
try 26 'int main(){1 + (2 * 3) * 4 + 1;}'
try 3 'int main(){-5 + 8;}'
try 100 'int main(){+100 + 0;}'
try 0 'int main(){4 == 1;}'
try 1 'int main(){1 == 1;}'
try 0 'int main(){8 != 8;}'
try 1 'int main(){0 == 0;}'
try 1 'int main(){1 < 2;}'
try 0 'int main(){100 < 5;}'
try 1 'int main(){913 <= 1000;}'
try 0 'int main(){1 <= 0;}'
try 1 'int main(){1 > 0;}'
try 0 'int main(){0 > 1;}'
try 1 'int main(){1 >= 0;}'
try 0 'int main(){0 >= 1;}'
try 1 'int main(){int a; a = 1;}'
try 123 'int main(){return 123;}'
try 1 'int main(){int abc; abc = 1;}'
try 2 'int main(){int a; a = 1 + 1; return a;}'
try 1 'int main(){int ab; int x; ab = 0; x = 1;}'
try 5 'int main(){int a; int b; a = 5; b = 10; return a;}'
# try 5 'int main(){int a, b; a = 5; b = 10; return a;}'
# try 5 'int main(){int a = 5; int b = 10; return a;}'
try 10 'int main(){int a; int b; a = 5; b = 10; return b;}'
try 10 'int main(){int a = 10; return a;}'
try 15 'int main(){int a; int b; a = 5; b = 10; return a + b;}'
try 24 'int main(){int a; int b; int c; a = 2; b = 3; c = 4; return a * b * c;}'
# [NOT WORKING] big int.
# try 1000 'int main(){a = 2; b = 50; c = 10; return a * b * c;}'
try 255 'int main(){int x; x = 0; return ~x;}' # TODO:負に対応していない
try 2 'int main(){int a; a = 1; if(a == 1){ return 2; }}'
try 2 'int main(){int a; a = 1; if(a == 1) return 2;}'
try 11 'int main(){int a;a = 1; if(1==a){return 11;}}'
try 10 'int main(){int a;a = 1; if(a == 1){ 1 + 1; return 10; } return 8;}'
try 8 'int main(){if(1 == 1) return 8;}'
try 5 'int main(){if(1 == 1){ int a;a = 3; return 5; }}'
try 9 'int main(){if(1 == 0){return 1;} else{ return 9;}}'
try 5 'int main(){if(1==1){if(2==2){return 5;}}}'
try 10 'int main(){if(1==1){if(1==2){return 5;}else{return 10;}}}'
try 1 'int main(){return 1 == 1 && 2 == 2;}'
try 5 'int main(){if(1 == 1 && 2 == 2) return 5;}'
try 1 'int main(){return 1 == 1 || 1 == 2;}'
try 5 'int main(){if(1 == 1 || 1 == 1) return 5;}'
try 5 'int main(){if(1 == 2 || 1 == 1) return 5;}'
try 5 'int main(){if(1 == 1 || 1 == 2) return 5;}'
try 5 'int main(){if(1 | 0){return 5;}else{return 10;}}'
try 10 'int main(){if(1 & 0){return 5;}else{return 10;}}'
try 10 'int main(){if (1 == 2 || 2 ==1){return 5;} return 10;}'
try 99 'int main(){int a;a = 5; if(1 == 1 && a == 5){return 99;}}'
try 4 'int main(){int a;a = 3; a+=1; return a;}'
try 4 'int main(){int a;a = 5; a-=1; return a;}'
try 4 'int main(){int a;a = 2; a*=2; return a;}'
try 4 'int main(){int a;a = 8; a/=2; return a;}'
try 4 'int main(){int a; a = 7; a&=4; return a;}'
try 7 'int main(){int a; a = 7; a|=4; return a;}'
try 3 'int main(){int a; a = 7; a^=4; return a;}'
try 6 'int main(){int a; a = 5; ++a; return a;}'
try 4 'int main(){int a; a = 5; --a; return a;}'
try 6 'int main(){int a; a = 5; a++; return a;}'
try 5 'int main(){int a; a = 5; return a++;}'
try 5 'int main(){int a; int b; a = 5; b = a++; return b;}'
try 6 'int main(){int a; int b; a = 5; b = a++; return a;}'
try 5 'int main(){int a; a = 5; return a--;}'
try 5 'int main(){int a; int b; a = 5; b = a--; return b;}'
try 4 'int main(){int a; int b; a = 5; b = a--; return a;}'
try 2 'int main(){return 12 % 5;}'
try 2 'int main(){int a; a = 12 % 10; return 2;}'
try 2 'int main(){int a; a = 12; a%=10; return a;}'
try 16 'int main(){int a; a = 2; return a << 3;}'
try 2 'int main(){int a; a = 16; return a >> 3;}'
try 16 'int main(){int a; a = 2; a <<= 3; return a;}'
try 2 'int main(){int a; a = 16; a >>= 3; return a;}'
try 3 'int main(){if(1==0){return 2;}else if(2 == 2){return 3;}else{return 4;}}'
try 4 'int main(){if(1==0){return 2;}else if(1 == 2){return 3;}else{return 4;}}'
try 10 'int main(){ if(1==0){return 2;} else if(1 == 0){return 5;} return 10;}'
try 2 'int main(){int a; a = 1; while(a == 1){ a += 1;} return a;}'
try 5 'int main(){int a; for(a = 1; a<5; a+=1){ a+=1;} return a;}'
try 1 'int main(){int a; a = 0; do{ a+=1;}while(a > 10); return a;}'
try 10 'int main(){int a; a = 0; do{ a+=1;}while(a < 10); return a;}'
try 10 'int main(){for(;;){return 10;}}'
try 10 'int main(){int a; for(a=10;;){return a;}}'
try 10 'int main(){int a;a = 10; for(;a<100;){return a;}}'
try 10 'int main(){int a;a = 10; for(;;a+=1){return a;}}'
try 100 'int main(){int a;a = 1; for(;a<100;){a+=1;} return a;}'
try 5 'int main(){return foo();}'
try 5 'int bar(){return 5;} int main(){return bar();}'
try 10 'int bar(){int a; a = 5; return a;} int main(){int a; a = 10; return a;}'
try 5 'int plus(int x, int y){return x + y;} int main(){ return plus(3,2);}'
try 10 'int main(){int x; int i; x = 0; for(i = 0; i < 10; i+=1){x+=1;} return x;}'
try 55 'int sum(int m, int n) { int acc; int i; acc = 0; for (i = m; i < n; i = i + 1) acc = acc + i; return acc;} int main() {return sum(0, 11);}'
try 30 'int rec(int n){if(n == 1){return 30;} return rec(1);} int main(){return rec(2);}'
try 4 'int bar(){return 2;} int main(){return bar() + bar();}'
try 30 'int bar(int n){return n;} int main(){return bar(10) + bar(20);}'
try 30 'int bar(int n){return n;} int main(){return bar(5+5) + bar(40/2);}'
try 18 'int bar(int n){return n;} int main(){int a; a = 5; return bar(a+a) + bar(40/a);}'
try 55 'int fib(int n){ if(n == 1 || n == 2) {return 1;} else {return fib(n-1) + fib(n-2);}} int main(){ return fib(10); }'
try 89 'int fib(int n){ if(n == 1 || n == 2) {return 1;} else {return fib(n-1) + fib(n-2);}} int main(){ return fib(11); }'
try 89 'int fib(int n){ if(n == 1 || n == 2) {return 1;} return fib(n-1) + fib(n-2);} int main(){ return fib(11); }'
try 5 'int main(){ int i; i = 0; for(; i < 10; i+=1){if(i == 5) break; } return i;}'
try 5 'int main(){ int i; i = 0; while(i < 10){if(i == 5) break; i+=1;} return i;}'
try 9 'int main(){ int a; int i; a = 0; for(i=0; i<10; i+=1){if(i == 5) {continue;} a+=1; } return a; }'
try 9 'int main(){ int a; int i; i = 0; a = 0; while(i<10){if(i == 5) {i+=1; continue;} i+=1; a+=1; } return a; }'
try 3 'int main(){int x; int *y; x = 3; y = &x; return *y;}'
try 3 'int main(){int **x; int *y; int z; int *a; z = 3; y = &z; x = &y; a = *x; return *a;}'
try 6 'int main(){int a; int *b; int **c; a = 6; b = &a; c = &b; return **c;}'
try 4 'int main(){int a; int b; a = 7; b = 4; return a & b;}'
try 100 'int main(){int a; int b; int cnt; cnt = 0; for(a = 0; a < 10; a+=1){ for(b = 0; b < 10; b+=1){ cnt++; }} return cnt;}'
try 5 'int main(){if(1==1){if(1==1){return 5;} return 4;} return 3;}'
try 4 'int main(){if(1==1){if(1==2){return 5;} return 4;} return 3;}'
try 3 'int main(){if(1==2){if(1==2){return 5;} return 4;} return 3;}'
try 12 'int main(){return 4 ^ 8;}'

try 8 'int main(){int *q; int *p; p = alloc4(); q = p + 3; return *q;}'
try 2 'int main(){int *p; int *q; p = alloc4(); q = p + 1; return *q;}'
try 2 'int main(){int *p; p = alloc4(); p = p + 2; p = p - 1; return *p;}'
try 8 'int main(){int **p; p = alloc_ptr4(); p = p + 3; return **p;}'
try 4 'int main(){return sizeof(10);}'
try 4 'int main(){return sizeof(+10);}'
try 4 'int main(){return sizeof(-10);}'
try 4 'int main(){return sizeof(int);}'
try 4 'int main(){int a; return sizeof(a);}'
try 4 'int main(){int a; return sizeof a;}'
try 8 'int main(){int *a; return sizeof a;}'
try 4 'int main(){int a; return sizeof(a);}'
try 8 'int main(){int *a; return sizeof(a);}'
try 8 'int main(){return sizeof(int*);}'
try 40 'int main(){int a[10]; return sizeof(a);}'

try 10 'int main(){int a; a = 1; return a ? 10 : 5;}'
try 5 'int main(){int a; a = 0; return a ? 10 : 5;}'
try 10 'int main(){int a[10]; a[2] = 10; return a[2];}'
try 77 'int main(){int a[10]; a[9] = 77; return a[9];}'
try 55 'int main(){int a[10]; a[0] = 55; return a[0];}'
try 7 'int main(){int a[2]; *a = 7; return *a;}'
try 7 'int main(){int a[2]; *(a+1) = 7; return *(a+1);}'
try 1 'int main(){int a[2]; int *p; *a = 1; *(a + 1) = 2; p = a; return *p;}'
try 2 'int main(){int a[2]; int *p; *a = 1; *(a + 1) = 2; p = a; return *(p + 1);}'
try 3 'int main(){int a[2]; int *p; *a = 1; *(a + 1) = 2; p = a; return *p + *(p + 1);}'

echo ok
# rare case(can compile...)
# *(a + *(p = a)) = 2 + 1 << 2 >> 2; 左辺にも式を書くことが可能.
