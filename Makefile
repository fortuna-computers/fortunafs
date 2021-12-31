CPPFLAGS = -std=c11 -Wall -Wextra -Isrc

mkfs.fortunafs: src/mkfs.o src/fortunafs.o
	gcc $^ -o $@

clean:
	rm -f *.o **/*.o mkfs.fortunafs
.PHONY: clean
