OPTIMIZE = -g
CC = g++
CC_FLAGS = -std=c++20


main.exe : main.o bplus.o visualization.o
	$(CC) -o $@ $^ $(CC_FLAGS) -luser32 -lgdi32

main.o : main.cpp bplus.h
	$(CC) -c -o $@ $< $(OPTIMIZE) $(CC_FLAGS)

bplus.o : bplus.c bplus.h
	$(CC) -c -o $@ $< $(OPTIMIZE) $(CC_FLAGS)

visualization.o : visualization.cpp visualization.h
	$(CC) -c -o $@ $< $(OPTIMIZE) $(CC_FLAGS)

.PHONY: clean

clean :
	$(RM) *.o *.exe
