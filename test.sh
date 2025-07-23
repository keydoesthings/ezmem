gcc -c ezmem.c
ar rcs libezmem.a ezmem.o
rm ezmem.o
gcc -o test test.c -L. -lezmem -std=c89
./test