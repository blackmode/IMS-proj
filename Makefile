
CPP=g++

all: ims-svoz-odpadu.o
	$(CPP) -o ims-svoz-odpadu ims-svoz-odpadu.o -lsimlib -lm
	rm -f *.o
	
clean:
	rm -f ims-svoz-odpadu

run: 
	./ims-svoz-odpadu
	
test:
	./ims-svoz-odpadu
	cat stats.out

