//  model  MODEL1
#include "simlib.h"
#include <stdio.h>
#include <limits.h>
#include <ctime>


//  deklarace  globálních  objektů
Facility  Box("Linka");
Histogram Tabulka("Tabulka",0,50,10);

class Zakaznik : public Process { // třída zákazníků
  double Prichod;                 // atribut každého zákazníka
  void Behavior() {               // popis chování zákazníka
    Prichod = Time;               // čas příchodu zákazníka
    Seize(Box);                   // obsazení zařízení Box
    Wait(10);                     // obsluha
    Release(Box);                 // uvolnění
    Tabulka(Time-Prichod);        // doba obsluhy a čekání
  }
};

class Generator : public Event {  // generátor zákazníků
  void Behavior() {               // popis chování generátoru
    (new Zakaznik)->Activate();     // nový zákazník v čase Time
    Activate(Time+Exponential(1e3/150)); // interval mezi příchody
  }
};

//  popis  experimentu
int main()
{
  Print("***** MODEL1 *****\n");
  Init(0,1000);              // inicializace experimentu
 // for (int i=0; i < 4; i++)
     (new Generator)->Activate(); // generátor zákazníků, aktivace
  Run();                     // simulace
  Box.Output();              // tisk výsledků
  Tabulka.Output();
  return 0;
}
