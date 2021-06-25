# Makefile
all: main merge qsort
clean:
	rm main merge qsort
main:
	gcc  main.c -o main
qsort:
	gcc  qsort.c -o qsort
merge:
	gcc  merge.c -o merge