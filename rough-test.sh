#!/bin/bash
make
./9cc "a=1;b=2;a*b;" | tee tmp.s
cc -o tmp tmp.s
./tmp
echo "$?"