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
assert 6 "3*2"
assert 6 "1*2*3"
assert 7 " 4 + (2 - 1)*3"
assert 3 "6/3+1"
assert 1 "-1+2"

assert 1 "2==2"
assert 0 "2==3"
assert 1 "2!=3"
assert 0 "2!=2"

assert 1 "1<2"
assert 0 "3<2"

assert 0 "1>2"
assert 1 "3>2"

assert 1 "1<=1"
assert 1 "1<=2"
assert 0 "3<=2"

assert 1 "1>=1"
assert 1 "2>=1"
assert 0 "2>=3"

echo OK