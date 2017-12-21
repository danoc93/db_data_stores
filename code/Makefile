# Makefile
CC = g++ -std=c++11 
 
library.o: library.cpp library.hpp
	$(CC) -o $@ -c $<

write_fixed_len_pages: write_fixed_len_pages.cpp write_fixed_len_pages.hpp library.o
	$(CC) -o $@ $< library.o
 
read_fixed_len_page: read_fixed_len_page.cpp read_fixed_len_page.hpp library.o
	$(CC) -o $@ $< library.o

csv2heapfile: csv2heapfile.cpp library.o
	$(CC) -o $@ $< library.o
 
scan: scan.cpp library.o
	$(CC) -o $@ $< library.o
 
insert: insert.cpp library.o
	$(CC) -o $@ $< library.o

update: update.cpp library.o
	$(CC) -o $@ $< library.o

delete: delete.cpp library.o
	$(CC) -o $@ $< library.o

select: select.cpp library.o
	$(CC) -o $@ $< library.o

csv2colstore: csv2colstore.cpp library.o
	$(CC) -o $@ $< library.o

select2: select2.cpp library.o
	$(CC) -o $@ $< library.o

select3: select3.cpp library.o
	$(CC) -o $@ $< library.o

clean:
	rm -rf *.o

move:
	mv write_fixed_len_pages read_fixed_len_page csv2heapfile scan insert update delete select csv2colstore select2 select3 ../executables
	rm -rf *.o

all: write_fixed_len_pages read_fixed_len_page csv2heapfile scan insert update delete select csv2colstore select2 select3
