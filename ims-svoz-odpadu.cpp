//  model  MODEL1
#include "simlib.h"
#include <stdio.h>
#include <limits.h>
#include <ctime>
#include <iostream>
#include <algorithm>

#define DEBUG_MODE 1

// počet ULIC
#define POCET_ULIC 16
#define MAX_POCET_ULIC_DEN 10 // Kolik ulic bude maximalne na den zpracovavat jedno auto

#define DEPO 9 		//  uzel(KRIZOVATKA) na kterem se nachazi depo odkud vyrazi auto
#define SKLADKA 10	//  uzel(SKLADKA) na kterem se nachazi skladka kam jede auto vyvezt odpad 

// počet křižovatek
#define POCET_KRIZOVATEK 11

#define PRUMERNA_RYCHLOST_PRESUNU_MEZI_ULICEMI 11 // m/s
#define PRUMERNA_RYCHLOST_PRESUNU_MEZI_DOMY 3 // m/s

// CASOVE KONSTANTY
const int MINUTA = 60; // SEKUND
const int HODINA = MINUTA*60; // hodina
const int DEN = HODINA*24; // den
const int TYDEN = DEN*7; // tyden

// KONSTANTY ULIC
// jedna se o sloupec v matici NxN tj. ktery sloupec je ktery
const int ULICE_ID = 0;
const int ULICE_DELKA = 1;
const int ULICE_POCET_DOMU = 2;
const int ULICE_TYP_ULICE = 3;		// NORMAL/JEDNOSMERKA 
const int ULICE_TYP_ZASTAVBY = 4;	// bytovka/rodinny dum/panelak apod.
const int ULICE_KRIZOVATKA_X = 5;	// kazda ulice ci jeji cast je definovana krizovatkou/uzlem X 
const int ULICE_KRIZOVATKA_Y = 6;	// kazda ulice ci jeji cast je definovana krizovatkou/uzlem Y

// jakej je zrovne den v tydnu
int den_v_tydnu = -1;

// je vykend ci nikoliv
int je_vikend = 0;


/** 
* modelovana oblast => graf krizovatek 
* POKUD JEDNA ULICE BUDE MIT VICE JAK DVA UZLY (bude nutne ji rozdelit) tak ty uzly pojmenovavat postupne tak,
* aby, nejnizsi ID uzlu byl zacatek/konec ulice a nejvyssi ID uzlu konec/zacatek ALE NE Prostredek
* matice sousednosti
*/
int graph[POCET_KRIZOVATEK][POCET_KRIZOVATEK] = {
	//  0  1  2  3  4  5  6  7  8  9  10
	   {0, 4, 0, 0, 0, 0, 0, 8, 0, 0, 0},	// 0
	   {4, 0, 8, 0, 0, 0, 0, 11, 0, 0, 0},	// 1
	   {0, 8, 0, 7, 0, 4, 0, 0, 2, 0, 0},	// 2
	   {0, 0, 7, 0, 9, 14, 0, 0, 0, 0, 0},	// 3
	   {0, 0, 0, 9, 0, 10, 0, 0, 0, 0, 14},	// 4
	   {0, 0, 4, 0, 10, 0, 2, 0, 0, 0, 0},	// 5
	   {0, 0, 0, 14, 0, 2, 0, 1, 6, 20, 0},	// 6
	   {8, 11, 0, 0, 0, 0, 1, 0, 7, 0, 0},	// 7
	   {0, 0, 2, 0, 0, 0, 6, 7, 0, 0, 0},	// 8
	   {0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0},	// 9
	   {0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0}	// 10
};

/** 
* modelovana oblast => popis ulic
* ZDE DEFINUJEME, VSECHNY POTREBNY UDEJA O ULICICH, jako je pocet domu, delka ulice, napojueni na krizovatky
* ulice pod stejnym ID znamena, ze se jedna o jednu a tuttez ulici, ktera je napohjena na vice krizovatek
* proste ma vic uzlu...
* Popis sloupců: { ID ULICE , DELKA ULICE V METRECH ,počet domů ,typ zastavby ,typ ulice , křižovatka X , křižovatka Y }
*/
int ulice[POCET_ULIC][7] = {
  {1,7,4,1,0,7,8},

  {2,1,1,1,0,7,6},
  {2,2,2,1,0,6,5},

  {3,4,5,1,0,0,1},
  {3,8,6,1,0,1,2},
  {3,7,4,1,0,2,3},

  {4,4,6,1,0,2,5},

  {5,9,11,1,0,3,4},

  {6,10,8,1,0,5,4},

  {7,8,9,1,0,0,7},

  {8,11,15,1,0,1,7},

  {9,14,20,1,0,3,5},

  {10,2,1,1,0,2,8},
  {10,6,4,1,0,8,6},
//------------------------------------------ zde se nic nezpracovava
  {11,20,0,0,0,6,9}, // depo

  {12,14,0,0,0,4,10} // skladka
};

// Vysledne pole pro nejratsi cestu
int nejkratsi_cesta[POCET_KRIZOVATEK] = {0,0,0,0,0,0,0,0,0};

//  deklarace  globálních  objektů
Facility  Skladka("Skládka odpadu");
Histogram Tabulka("Tabulka",0,50,10);


class Auto : public Process {
	double Prichod;                 // atribut každého Auta
	int ujeta_vzdalenost;           // atribut každého Auta
	int trasy_auto[5][MAX_POCET_ULIC_DEN];

	public: 
		Auto (int trasy[5][MAX_POCET_ULIC_DEN]) {
			// ujeta vzdalenost v mterech
			ujeta_vzdalenost = 0;

			// trasy ke zpracovani pro cely tyden
			for (int i = 0; i < 5; ++i){
				for (int j = 0; j < MAX_POCET_ULIC_DEN; ++j)
				{
					trasy_auto[i][j] = trasy[i][j];
				}
			}
		}


	void Behavior() {                 // popis chování auta
		Prichod = Time;               // čas vyjeti auta z depa

		// ja pracovni tyden
		if (!je_vikend) 
		{
			//while(1) {}
			int zpracovano_ulic = 0;
			int src = DEPO;				  		// odkud jsme vyjeli
			int start_end_nodes[2] = {-1,-1};	// urcuje zacatek a konec ulice

			// zpracovani ulic pro dany den
			while(zpracovano_ulic < MAX_POCET_ULIC_DEN and trasy_auto[den_v_tydnu][zpracovano_ulic]!=-1) 
			{
				int id_ulice = trasy_auto[den_v_tydnu][zpracovano_ulic];
				for (int i = 0; i < POCET_ULIC; ++i)
				{
					if (ulice[i][0] == id_ulice) 
					{
						// ulozeni prvniho uzlu
						if (start_end_nodes[0]==-1 && start_end_nodes[1]==-1) {
							start_end_nodes[0] = ulice[i][ULICE_KRIZOVATKA_X];
							start_end_nodes[1] = ulice[i][ULICE_KRIZOVATKA_Y];
						}
						// hledam konec ulice
						else {
							if (start_end_nodes[0] == ulice[i][ULICE_KRIZOVATKA_X]) {
								start_end_nodes[0] = ulice[i][ULICE_KRIZOVATKA_Y];
							}
							else if (start_end_nodes[0] == ulice[i][ULICE_KRIZOVATKA_Y]) {
								start_end_nodes[0] = ulice[i][ULICE_KRIZOVATKA_X];
							}
							else if (start_end_nodes[1] == ulice[i][ULICE_KRIZOVATKA_X]) {
								start_end_nodes[1] = ulice[i][ULICE_KRIZOVATKA_Y];
							}
							else if (start_end_nodes[1] == ulice[i][ULICE_KRIZOVATKA_Y]) {
								start_end_nodes[1] = ulice[i][ULICE_KRIZOVATKA_X];
							}
						}

					}
				}
				// zvolim si jako zacatek prvni parametr -> tady bych modl dat i nahodny vyber
				// jakoze ridic si voli trasu
				int dest = start_end_nodes[0];

				// presunu se na misto urceni, kde zacu zpracovavat ulici
				// src je zde porad to nase depo!
				presun(src,dest,&ujeta_vzdalenost);

				// jsem v ulici, kde mam zacit delat  dum po domu
				// zde bych mel zacit zpracovavat ulici dum po domu
				for (int i = 0; i < POCET_ULIC; ++i)
				{
					// zjstim zdali jsem v te ulici resp, jestli jsem na tom useku kde mam
					if (ulice[i][0] == id_ulice) 
					{
						if (get_street_by_endpoint(ulice,id_ulice,dest )!=-1) 
						{
							int id_zpracovavane_ulice = get_street_by_endpoint(ulice,id_ulice,dest );
							int pocet_domu_ulice = ulice[id_zpracovavane_ulice][ULICE_POCET_DOMU];
							int delka_useku_ulice = ulice[id_zpracovavane_ulice][ULICE_DELKA];
							int typ_zastavby = ulice[id_zpracovavane_ulice][ULICE_TYP_ZASTAVBY]; // pocet popelnice

							// musim overit, jestli auto neni plne
							// pokud ne, tak zpracovavam
							int i_zprac = pocet_domu_ulice;
							while (i_zprac>0) 
							{
								int doba_prijezdu_k_domu = (delka_useku_ulice/pocet_domu_ulice)/PRUMERNA_RYCHLOST_PRESUNU_MEZI_DOMY;
								Wait(doba_prijezdu_k_domu);
								// zpracovani popelnice  v prumeru 30s/popelnice
								Wait(45*typ_zastavby);
								i_zprac--;
							}
							// usek mam hotov, posunu se dal
							src = dest; // jsem v src
							// jedu do dest
							dest = get_next_point(ulice,id_ulice,dest );
							printf("domu: %d\n", pocet_domu_ulice);
							printf("Jedu do: %d\n", dest);
							printf("Jedu do: %d\n", dest);
						}
					}
				}



				
				break;
				// Musim nacist vsechny uzly pro danou ulici
				// a vyhodit dest misto
 				zpracovano_ulic++;
			}

		}


		/*	Wait(10);                     // obsluha V
		
			dijkstra(graph, 8,5);
			p(nejkratsi_cesta, POCET_KRIZOVATEK);

			dijkstra(graph, 0,5);
			p(nejkratsi_cesta, POCET_KRIZOVATEK);

			dijkstra(graph, 3,6);
			p(nejkratsi_cesta, POCET_KRIZOVATEK);

			printf("\n\n\n\n => |%d|", get_street(ulice, 6,9));
		*/
	}

  	// presun mezi krizovatkami
	int presun(int src, int dest, int* ujeta_vzdalenost) 
	{
		int vzdalenost = dijkstra(graph, src,dest);
		Wait(vzdalenost/PRUMERNA_RYCHLOST_PRESUNU_MEZI_ULICEMI);
		*ujeta_vzdalenost = *ujeta_vzdalenost + vzdalenost;
		return vzdalenost;
	}



  // debug print: vypise pole int o urcite velikosti
  void p(int pole[], int velikost) {
	  printf("\n");
	  for (int i = 0; i < velikost; i++)
	  {
		  printf("%d ",pole[i]);
	  }
	  printf("\n");
  }


  // A utility function to find the vertex with minimum distance
  // value, from the set of vertices not yet included in shortest
  // path tree
  int minDistance(int dist[], bool sptSet[])
  {
	  // Initialize min value
	  int min = INT_MAX, min_index;
   
	  for (int v = 0; v < POCET_KRIZOVATEK; v++)
		  if (sptSet[v] == false && dist[v] <= min)
			  min = dist[v], min_index = v;
   
	  return min_index;
  }
   
	  // Function to print shortest path from source to j
	  // using parent array
	  // DEBUG FUNKCE
	void printPath(int parent[], int j, int src)
	{
		int k = j;
		//int src = 0;
		int c = 0;
		int i = 0;

		// init
		for ( i = 0;i<POCET_KRIZOVATEK; i++)
			nejkratsi_cesta[i] = -1;

		// vygenerovani vysledne cesty, bude ale invertovana
		// takze ju jeste musime otocit
		while(k<=POCET_KRIZOVATEK && parent[k]!=src && k!=-1) 
		{
			//printf("%d ", k);  //V 
			nejkratsi_cesta[c] = k;
			k = parent[k];
			c++;

			// DEBUG
			if (DEBUG_MODE)
				if (c>200) {
					printf("Doslo k zacykleni!!!\n");
					break;
				}
		}
		// zde jeste musim ulouzit posledni uzel cesty
		nejkratsi_cesta[c] = k;
		//printf("%d ", k);  

		// zde bude ulozen index pole, kde konci vysledna cesta v poli: nejkratsi_cesta[]
		// aby se pak mohla invertovat
		int count;

		for (count = 0;count<POCET_KRIZOVATEK; count++)
		{
			// jakmile jsem narazil na -1 tak jsem na konci cesty
			if ( nejkratsi_cesta[count]==-1) {
				if (nejkratsi_cesta[count-1]!=src) {
					nejkratsi_cesta[count] = src;
					break;
				}

				count--;
				break;
			}
		}

		// deklarace podle toho, jak je cesta dlouha
		int temp[count+1];

		// init docasneho pole
		for (int ti = 0;ti<count+1; ti++)
		   temp[ti] = -1;

		// obraceni pole
		for (int i = count,h=0; i>=0; i--, h++)
			temp[h] = nejkratsi_cesta[i];

		// reset vysledneho pole
		for ( i = 0;i<POCET_KRIZOVATEK; i++)
		   nejkratsi_cesta[i] = -1;

		// vypis pole
		for (i = 0;i<count+1; i++)
		{
			nejkratsi_cesta[i] = temp[i];
			if (DEBUG_MODE)
				printf("%d ", temp[i]);
		}

		if (DEBUG_MODE)
			printf("|%d|", count);
	 }


  // A utility function to print the constructed distance
  // array
  // DEBUG FUNKCE
  int printAllPath(int dist[], int n, int parent[],int src, int dest)
  {
	  printf("Vertex\t\t  Distance\t\tPath");
	  for (int i = 0; i < POCET_KRIZOVATEK; i++)
	  {
		  printf("\n%d -> %d \t\t\t %d\t\t ", src, i, dist[i]);
		  printPath(parent, i, src);
	  }
  }


  // Funtion that implements Dijkstra's single source shortest path
  // algorithm for a graph represented using adjacency matrix
  // representation
  int dijkstra(int graph[POCET_KRIZOVATEK][POCET_KRIZOVATEK], int src, int dest)
  {
		int dist[POCET_KRIZOVATEK];  // The output array. dist[i] will hold
					// the shortest distance from src to i

		// sptSet[i] will true if vertex i is included / in shortest
		// path tree or shortest distance from src to i is finalized
		bool sptSet[POCET_KRIZOVATEK];

		// Parent array to store shortest path tree
		int parent[POCET_KRIZOVATEK];
		   
		for (int i = 0;i<POCET_KRIZOVATEK; i++)
			parent[i] = -1;
		
		// Initialize all distances as INFINITE and stpSet[] as false
		for (int i = 0; i < POCET_KRIZOVATEK; i++)
		{
		  parent[0] = -1;
		  dist[i] = INT_MAX;
		  sptSet[i] = false;
		}

		// Distance of source vertex from itself is always 0
		dist[src] = 0;

		// Find shortest path for all vertices V
		for (int count = 0; count < POCET_KRIZOVATEK-1; count++)
		{
		  // Pick the minimum distance vertex from the set of
		  // vertices not yet processed. u is always equal to src
		  // in first iteration.
		  int u = minDistance(dist, sptSet);

		  // Mark the picked vertex as processed
		  sptSet[u] = true;

		  // Update dist value of the adjacent vertices of the
		  // picked vertex.
		  for (int v = 0; v < POCET_KRIZOVATEK; v++)

			  // Update dist[v] only if is not in sptSet, there is
			  // an edge from u to v, and total weight of path from
			  // src to v through u is smaller than current value of
			  // dist[v]
			  if (!sptSet[v] && graph[u][v] &&
				  dist[u] + graph[u][v] < dist[v])
			  {
				  parent[v]  = u;
				  dist[v] = dist[u] + graph[u][v];
			  }  
		}
		if (DEBUG_MODE) {
			printf("Vertex\t\t  Distance\t\tPath");
			printf("\n%d -> %d \t\t\t %d\t\t ", src, dest, dist[dest]);
		}

		printPath(parent, dest, src);
		return dist[dest];
  }

  	// podle zadane casti ulice zjistim jeji ID
	int get_street(int streets[][7], int src, int dest) 
	{
		for (int i = 0; i < POCET_ULIC; ++i)
		{
			if ( (streets[i][5] == src && streets[i][6] == dest) || (streets[i][6] == src && streets[i][5] == dest)) {
				return i;
			}
		}
		return -1;
	}
  	// vraci cast ulice, ktera navazuje na uzel nzadanej pramaterem: begin_end_point
	int get_street_by_endpoint(int streets[][7], int id_street, int begin_end_point) 
	{
		for (int i = 0; i < POCET_ULIC; ++i)
		{
			if (streets[i][0]==id_street) 
				if ( (streets[i][ULICE_KRIZOVATKA_X] == begin_end_point || streets[i][ULICE_KRIZOVATKA_Y] == begin_end_point)) 
					return i;
		}
		return -1;
	}
  	// vraci dalsi uzel ulice id_street, kterej je spojenej s begin_end_point uzlem
	int get_next_point(int streets[][7], int id_street, int begin_end_point) 
	{
		for (int i = 0; i < POCET_ULIC; ++i)
		{
			if (streets[i][0]==id_street) 
				if ( (streets[i][ULICE_KRIZOVATKA_X] == begin_end_point))
					return streets[i][ULICE_KRIZOVATKA_Y];

				if ((streets[i][ULICE_KRIZOVATKA_Y] == begin_end_point)) 
					return streets[i][ULICE_KRIZOVATKA_X];
		}
		return -1;
	}

};

class Generator : public Event {  // generátor zákazníků
	int trasy_auto[5][MAX_POCET_ULIC_DEN];

	public:
		Generator(int trasy[5][MAX_POCET_ULIC_DEN]) {
			for (int i = 0; i < 5; ++i)
			{
				for (int j = 0; j < MAX_POCET_ULIC_DEN; ++j)
				{
					trasy_auto[i][j] = trasy[i][j];
				}
			}
		}

	void Behavior() {               		// popis chování generátoru
		(new Auto(trasy_auto))->Activate();      // nový zákazník v čase Time
		//Activate(Time+Exponential(1e3/150));  // interval mezi příchody
	}
};


// stridani dne v tydnu
class Gen_den : public Event { 

  void Behavior() { 
  	if (DEBUG_MODE) {
  		//printf("\nDen v tydnu: %d , je vikend: %d", den_v_tydnu, je_vikend);
  	}
	if (den_v_tydnu >=0 && den_v_tydnu<5) 
	{
		den_v_tydnu++;
		if (den_v_tydnu==5) {
			je_vikend = 1;
		}
		Activate(Time+DEN);
	}
	else if (den_v_tydnu==5 || den_v_tydnu==6) {
		
		// pokud je nedele, tak dalsi den je pondeli => 0 
		if (den_v_tydnu==6){
			if (DEBUG_MODE) {
				//printf("\n----------\n ");
			}
			den_v_tydnu=0;
			je_vikend = 0;
		}
		else {
			den_v_tydnu++;
		}
		Activate(Time+DEN);
	}
	else {
		// init
		den_v_tydnu = 0;
		Activate(Time+DEN);
	}

  }
};



//  popis  experimentu
int main()
{
	// POZN: pokud je pole vetsi nez datovej vstup, tak zbytek pole dopisem vzdycky  hodnotou -1 !!! 

  Print("***** MODEL1 *****\n");
  Init(0,5*TYDEN);              // inicializace experimentu

  // stridani dnu v tydnu
  (new Gen_den)->Activate(); 

  	// jake trasy pojede auto A
	int trasy_A[5][MAX_POCET_ULIC_DEN] = {
		{3,2,1,4,5,6,11,7,8,9},
		{1,2,3,4,5,6,11,7,8,9},
		{1,2,3,4,5,6,11,7,8,9},
		{1,2,3,4,5,6,11,7,8,9},
		{1,2,3,4,5,6,11,7,8,9}
	};

	// popelarsky vuz A
  (new Generator(trasy_A))->Activate(); // generátor zákazníků, aktivace
  Run();                     // simulace
  Skladka.Output();              // tisk výsledků
  Tabulka.Output();

  return 0;
}
