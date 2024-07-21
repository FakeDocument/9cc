#!/bin/bash
assert() {

    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

echo テスト開始
assert 2 "a=1;b=2;hoge=a*b;return hoge;"
assert 4 "a=2;if(a==2)a=4;return a;"
assert 0 "a=1;if(a==2)a=4;else a=0; return a;"
assert 5 "a=0;while(a<5)a=a+1;return a;"
assert 5 "a=0;for(i=0;i<5;i=i+1)a=a+1;return a;"
assert 5 "a=0;i=0;for(;i<5;i=i+1)a=a+1;return a;"
assert 10 "a=0;for(i=0;i<5;i=i+1){a=a+1;a=a+1;}return a;"
echo OK