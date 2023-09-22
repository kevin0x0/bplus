OPTIMIZE = -g


main.exe : main.o bplus.o
	gcc -o $@ $^

main.o : main.c bplus.h
	gcc -c -o $@ $< $(OPTIMIZE)

bplus.o : bplus.c bplus.h
	gcc -c -o $@ $< $(OPTIMIZE)
