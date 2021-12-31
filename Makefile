CPPFLAGS = -std=c11 -Wall -Wextra -Isrc

all: mkfs.fortunafs

mkfs.fortunafs: CPPFLAGS += -ggdb -O0
mkfs.fortunafs: src/mkfs.o src/fortunafs.o
	gcc $^ -o $@

clean:
	rm -f *.o **/*.o mkfs.fortunafs
.PHONY: clean
