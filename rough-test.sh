#!/bin/bash
make
./9cc "a=2;if(a==2)a=4;return a;" | tee tmp.s
cc -o tmp tmp.s
./tmp
echo "$?"