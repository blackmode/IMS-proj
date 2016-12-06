
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

zip:
	rm -f 06_xsluns01_xpokor63.zip 
	zip 06_xsluns01_xpokor63.zip $(PROGRAM).cpp Makefile dokumentace.pdf

github:
	git pull origin master
	git commit -am "update"
	git push origin master

again:
	rm -f $(PROGRAM)
	rm -f *.o
	make all
	./$(PROGRAM)