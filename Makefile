
CPP=g++
PROGRAM = ims-svoz-odpadu

all: $(PROGRAM).o
	$(CPP) -o $(PROGRAM) $(PROGRAM).o -lsimlib -lm
	rm -f *.o
	
clean:
	rm -f $(PROGRAM)

run: 
	./$(PROGRAM)
	
test:
	./$(PROGRAM)
	cat stats.out

