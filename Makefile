all: crtime

crtime: crtime.o
	gcc -o $@ $^ -lext2fs

test: crtime
	./test.sh

install: crtime
	cp $< /usr/local/bin/
