all: crtime

crtime: crtime.o
	gcc -o $@ $^ -lext2fs

test: crtime
	./test.sh
