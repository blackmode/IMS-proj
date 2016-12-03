
CPP=g++
PROGRAM = ims-svoz-odpadu
STATS = stats.out

all: $(PROGRAM).o
	$(CPP) -o $(PROGRAM) $(PROGRAM).o -lsimlib -lm -Wall -Wextra
	
clean:
	rm -f $(PROGRAM)
	rm -f *.o
	
run: 
	./$(PROGRAM)
	
test:
	./$(PROGRAM)
	cat $(STATS)

memtest:
	valgrind --leak-check=full -v --show-leak-kinds=all ./$(PROGRAM)

