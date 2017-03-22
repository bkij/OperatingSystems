CC=gcc
CFLAGS=-Wall -std=c99

.PHONY: clean

all: catalogue_scraper_nftw catalogue_scraper_dir

catalogue_scraper_dir: catalogue_scraper_dir.c
	$(CC) $(CFLAGS) $^ -o $@.out

catalogue_scraper_nftw: catalogue_scraper_nftw.c
	$(CC) $(CFLAGS) $^ -o $@.out

clean: 
	rm -f *.o *.out
