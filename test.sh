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
assert 2 "1+1"
assert 5 " 4 + 2 - 1"

echo OK