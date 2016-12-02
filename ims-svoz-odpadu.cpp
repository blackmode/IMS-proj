//  model  MODEL1
#include "simlib.h"
#include <stdio.h>
#include <limits.h>
#include <ctime>

// počet křižovatek
#define V 9

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
* Popis sloupců: { ID ULICE , DELKA ULICE V METRECH , počet domů , křižovatka X , křižovatka Y }
*/
int ulice[3][6] = {
  {1,500,34,2,5,6},
  {2,50,24,2,5,6},
  {2,70,6,2,5,6}
};


//  deklarace  globálních  objektů
Facility  Box("Linka");
Histogram Tabulka("Tabulka",0,50,10);






class Auto : public Process {       // třída zákazníků
  double Prichod;                 // atribut každého zákazníka

  void Behavior() {               // popis chování zákazníka
    Prichod = Time;               // čas příchodu zákazníka
    Wait(10);                     // obsluha
    
    dijkstra(graph, 0);
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
  void printPath(int parent[], int j)
  {
      // Base Case : If j is source
      if (parent[j]==-1)
          return;
   
      printPath(parent, parent[j]);
   
      printf("%d ", j);
  }
   
  // A utility function to print the constructed distance
  // array
  // DEBUG FUNKCE
  int printSolution(int dist[], int n, int parent[])
  {
      int src = 0;
      printf("Vertex\t  Distance\tPath");
      for (int i = 1; i < V; i++)
      {
          printf("\n%d -> %d \t\t %d\t\t%d ", src, i, dist[i], src);
          printPath(parent, i);
      }
  }
   
  // Funtion that implements Dijkstra's single source shortest path
  // algorithm for a graph represented using adjacency matrix
  // representation
  void dijkstra(int graph[V][V], int src)
  {
      int dist[V];  // The output array. dist[i] will hold
                    // the shortest distance from src to i
   
      // sptSet[i] will true if vertex i is included / in shortest
      // path tree or shortest distance from src to i is finalized
      bool sptSet[V];
   
      // Parent array to store shortest path tree
      int parent[V];
   
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
   
      // print the constructed distance array
      printSolution(dist, V, parent);
  }


};

class Generator : public Event {  // generátor zákazníků
  void Behavior() {               // popis chování generátoru
    (new Auto)->Activate();     // nový zákazník v čase Time
    //Activate(Time+Exponential(1e3/150)); // interval mezi příchody
  }
};

//  popis  experimentu
int main()
{


 




  Print("***** MODEL1 *****\n");
  Init(0,1000);              // inicializace experimentu
  (new Generator)->Activate(); // generátor zákazníků, aktivace
  Run();                     // simulace
  Box.Output();              // tisk výsledků
  Tabulka.Output();
  return 0;
}
