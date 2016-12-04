//  model  MODEL1
#include "simlib.h"
#include <stdio.h>
#include <limits.h>
#include <ctime>

#define DEBUG_MODE 1

// počet ULIC
#define POCET_ULIC 50
#define MAX_POCET_ULIC_DEN 10 // Kolik ulic bude maximalne na den zpracovavat jedno auto


// počet ULIC
#define POCET_KRIZOVATEK 50

// počet křižovatek => provizorni
#define V 9

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
* matice sousednosti
*/
int graph[V][V] = {{0, 4, 0, 0, 0, 0, 0, 8, 0},
				   {4, 0, 8, 0, 0, 0, 0, 11, 0},
				   {0, 8, 0, 7, 0, 4, 0, 0, 2},
				   {0, 0, 7, 0, 9, 14, 0, 0, 0},
				   {0, 0, 0, 9, 0, 10, 0, 0, 0},
				   {0, 0, 4, 0, 10, 0, 2, 0, 0},
				   {0, 0, 0, 14, 0, 2, 0, 1, 6},
				   {8, 11, 0, 0, 0, 0, 1, 0, 7},
				   {0, 0, 2, 0, 0, 0, 6, 7, 0}
				  };

/** 
* modelovana oblast => popis ulic
* ZDE DEFINUJEME, VSECHNY POTREBNY UDEJA O ULICICH, jako je pocet domu, delka ulice, napojueni na krizovatky
* ulice pod stejnym ID znamena, ze se jedna o jednu a tuttez ulici, ktera je napohjena na vice krizovatek
* proste ma vic uzlu...
* Popis sloupců: { ID ULICE , DELKA ULICE V METRECH , typ zastavby , počet domů , křižovatka X , křižovatka Y }
*/
int ulice[3][7] = {
  {1,500,2,34,2,5,6},
  {2,50,2,24,2,2,4},
  {2,70,2,6,2,9,6}
};

// Vysledne pole pro nejratsi cestu
int nejkratsi_cesta[V] = {0,0,0,0,0,0,0,0,0};

//  deklarace  globálních  objektů
Facility  Skladka("Skládka odpadu");
Histogram Tabulka("Tabulka",0,50,10);






class Auto : public Process {     // třída zákazníků
	double Prichod;                 // atribut každého zákazníka
	int ujeta_vzdalenost;           // atribut každého zákazníka

	public: 
		Auto (int trasy[5][MAX_POCET_ULIC_DEN]) {

		}


	void Behavior() {               // popis chování zákazníka
		Prichod = Time;               // čas příchodu zákazníka

		Wait(10);                     // obsluha
		
		dijkstra(graph, 8,5);
		p(nejkratsi_cesta, V);

		dijkstra(graph, 0,5);
		p(nejkratsi_cesta, V);

		dijkstra(graph, 3,6);
		p(nejkratsi_cesta, V);

		printf("\n\n\n\n => |%d|", get_street(ulice, 6,9));

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
   
	  for (int v = 0; v < V; v++)
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
		for ( i = 0;i<V; i++)
			nejkratsi_cesta[i] = -1;

		// vygenerovani vysledne cesty, bude ale invertovana
		// takze ju jeste musime otocit
		while(k<=V && parent[k]!=src && k!=-1) 
		{
			//printf("%d ", k);   
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

		for (count = 0;count<V; count++)
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
		for ( i = 0;i<V; i++)
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
	  for (int i = 0; i < V; i++)
	  {
		  printf("\n%d -> %d \t\t\t %d\t\t ", src, i, dist[i]);
		  printPath(parent, i, src);
	  }
  }


  // Funtion that implements Dijkstra's single source shortest path
  // algorithm for a graph represented using adjacency matrix
  // representation
  int dijkstra(int graph[V][V], int src, int dest)
  {
		int dist[V];  // The output array. dist[i] will hold
					// the shortest distance from src to i

		// sptSet[i] will true if vertex i is included / in shortest
		// path tree or shortest distance from src to i is finalized
		bool sptSet[V];

		// Parent array to store shortest path tree
		int parent[V];
		   
		for (int i = 0;i<V; i++)
			parent[i] = -1;
		
		// Initialize all distances as INFINITE and stpSet[] as false
		for (int i = 0; i < V; i++)
		{
		  parent[0] = -1;
		  dist[i] = INT_MAX;
		  sptSet[i] = false;
		}

		// Distance of source vertex from itself is always 0
		dist[src] = 0;

		// Find shortest path for all vertices
		for (int count = 0; count < V-1; count++)
		{
		  // Pick the minimum distance vertex from the set of
		  // vertices not yet processed. u is always equal to src
		  // in first iteration.
		  int u = minDistance(dist, sptSet);

		  // Mark the picked vertex as processed
		  sptSet[u] = true;

		  // Update dist value of the adjacent vertices of the
		  // picked vertex.
		  for (int v = 0; v < V; v++)

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


};

class Generator : public Event {  // generátor zákazníků
int trasyg[5][MAX_POCET_ULIC_DEN];

public:
	Generator(int trasy[5][MAX_POCET_ULIC_DEN]) {
		for (int i = 0; i < 5; ++i)
		{
			for (int j = 0; j < MAX_POCET_ULIC_DEN; ++j)
			{
				trasyg[i][j] = trasy[i][j];
			}
		}
	}

  void Behavior() {               // popis chování generátoru
	(new Auto(trasyg))->Activate();     // nový zákazník v čase Time
	//Activate(Time+Exponential(1e3/150)); // interval mezi příchody
  }
};


// stridani dne v tydnu
class Gen_den : public Event { 

  void Behavior() { 
  	if (DEBUG_MODE) {
  		printf("\nDen v tydnu: %d , je vikend: %d", den_v_tydnu, je_vikend);
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
				printf("\n----------\n ");
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

  Print("***** MODEL1 *****\n");
  Init(0,5*TYDEN);              // inicializace experimentu

  // stridani dnu v tydnu
  (new Gen_den)->Activate(); 

  	// jake trasy pojede auto A
	int trasy_A[5][MAX_POCET_ULIC_DEN] = {
		{1,2,3,4,5,6,11,23,32,33},
		{1,2,3,4,5,6,11,23,32,33},
		{1,2,3,4,5,6,11,23,32,33},
		{1,2,3,4,5,6,11,23,32,33},
		{1,2,3,4,5,6,11,23,32,33}
	};

	// popelarsky vuz A
  (new Generator(trasy_A))->Activate(); // generátor zákazníků, aktivace
  Run();                     // simulace
  Skladka.Output();              // tisk výsledků
  Tabulka.Output();

  return 0;
}
