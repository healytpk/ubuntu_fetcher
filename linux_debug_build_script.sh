#!/bin/sh
g++ -o prog -std=c++17 *.cpp -I/usr/include/json-c/ \
            -ljson-c -lcurl -ggdb3 -pedantic -Wall -Wextra -rdynamic \
            -funwind-tables -fno-omit-frame-pointer -fno-common -pthread \
            -fsanitize=address,leak,undefined,pointer-compare,pointer-subtract,float-divide-by-zero,float-cast-overflow \
            -fsanitize-address-use-after-scope -fcf-protection=full -fstack-protector-all -fstack-clash-protection \
            -fvtv-debug -fvtv-counts -finstrument-functions \
            -D_GLIBC_DEBUG -D_GLIBC_DEBUG_PEDANTIC -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
