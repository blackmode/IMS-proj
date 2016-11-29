#include "simlib.h"
#include <ctime>











//incializace a provedeni experimentu
int main() {

	RandomSeed(time(NULL));

	// inicializace experimentu
	Init(0,100);   

	// simulace
	Run();


	// NEjakej statistiky apod.
}

