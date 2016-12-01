
CPP=g++
PROGRAM = ims-svoz-odpadu
STATS = stats.out

all: $(PROGRAM).o
	$(CPP) -o $(PROGRAM) $(PROGRAM).o -lsimlib -lm
	
clean:
	rm -f $(PROGRAM)
	rm -f *.o
	
run: 
	./$(PROGRAM)
	
test:
	./$(PROGRAM)
	cat $(STATS)

