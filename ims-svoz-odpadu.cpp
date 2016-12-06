//  model  MODEL1
#include "simlib.h"
#include <stdio.h>
#include <limits.h>
#include <ctime>
#include <iostream>
#include <algorithm>

#define DEBUG_MODE 0
#define MAX_POCET_KRIZOVATEK_ULICE 10


// počet ULIC
#define POCET_ULIC 43
#define MAX_POCET_ULIC_DEN 10 // Kolik ulic bude maximalne na den zpracovavat jedno auto

#define DEPO 35 		//  uzel(KRIZOVATKA) na kterem se nachazi depo odkud vyrazi auto
#define SKLADKA 36	//  uzel(SKLADKA) na kterem se nachazi skladka kam jede auto vyvezt odpad 

// počet křižovatek
#define POCET_KRIZOVATEK 36

// rychlost auta
#define PRUMERNA_RYCHLOST_PRESUNU_MEZI_ULICEMI 11 // m/s
#define PRUMERNA_RYCHLOST_PRESUNU_MEZI_DOMY 3 // m/s

// doba zpracovani popelnice
#define DOBRA_ZPRACOVANI_POPELNICE 33 // s
#define DOBA_VYKLAPENI_ODPADU 921 // s

#define PRUMER_MNOZSTVI_ODPADU_CLOVEK 4 // KG

#define BULDOZER_RYCHLOST_ZPRACOVANI_TUNY_ODPADU 3600 // = HODINA
#define BULDOZER_KOLIK_ODPADU_ZPRACUJE_ZA_CAS 1000 // kg tunu zvladne

// doba zpracovani popelnice
#define MAX_KAPACITA_ODPADU_V_AUTE 11000 // kg => 11 tun

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

Queue Q1; // depo

// jakej je zrovne den v tydnu
int den_v_tydnu = -1;

// je vykend ci nikoliv
int je_vikend = 0;

int odpad_ke_zpracovani = 0;
int buldozer_aktivni = 0;

int svezeny_odpad = 0;
int celkova_ujeta_vzdalenost_auta = 0;

/** 
* modelovana oblast => graf krizovatek 
* POKUD JEDNA ULICE BUDE MIT VICE JAK DVA UZLY (bude nutne ji rozdelit) tak ty uzly pojmenovavat postupne tak,
* aby, nejnizsi ID uzlu byl zacatek/konec ulice a nejvyssi ID uzlu konec/zacatek ALE NE Prostredek
* matice sousednosti
*/
int graph2[POCET_KRIZOVATEK][POCET_KRIZOVATEK] = {
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

int graph[POCET_KRIZOVATEK][POCET_KRIZOVATEK] ={
{0,80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,335,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{80,0,110,0,0,0,0,0,225,0,0,0,0,0,0,0,0,0,0,158,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,110,0,150,105,0,0,230,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,150,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,105,0,0,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,70,0,75,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,75,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,230,0,0,0,0,0,76,219,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,225,0,0,0,0,0,76,0,0,0,0,0,0,194,320,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,219,0,0,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,128,0,110,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,110,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,211,0,0,90,176,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,194,0,0,0,176,0,0,171,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,320,0,0,0,0,0,171,0,128,126,0,0,0,254,0,0,0,0,0,0,0,0,0,0,0,145,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,126,0,0,55,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,55,0,100,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,158,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,100,0,112,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,112,0,93,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,0,93,0,345,0,0,0,0,0,0,0,0,0,0,0,0,0 },
{335,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,345,0,0,0,0,0,0,0,118,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,110,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,76,0,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,76,0,125,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,125,0,159,0,0,113,185,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,159,0,115,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,115,0,140,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,118,0,0,0,0,0,140,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,113,0,0,0,0,0,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,185,0,0,0,0,0,112,0,125,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,112,0,0,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,145,0,0,0,0,0,0,0,110,0,0,0,0,0,0,0,0,0,0,105,9423 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,125,0,105,0,0 },
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9423,0,0 }};


/** 
* modelovana oblast => popis ulic
* ZDE DEFINUJEME, VSECHNY POTREBNY UDEJA O ULICICH, jako je pocet domu, delka ulice, napojueni na krizovatky
* ulice pod stejnym ID znamena, ze se jedna o jednu a tuttez ulici, ktera je napohjena na vice krizovatek
* proste ma vic uzlu...
* Popis sloupců: { ID ULICE , DELKA ULICE V METRECH ,počet domů ,typ zastavby ,typ ulice , křižovatka X , křižovatka Y }
*/
// TEST GRAF
int ulice2[POCET_ULIC][7] = {
  {1,7,4,1,0,7,8},	// 0

  {2,1,1,1,0,7,6},	// 1
  {2,2,2,1,0,6,5},	// 2

  {3,4,5,1,0,0,1},	// 3
  {3,8,6,1,0,1,2},	// 4
  {3,7,4,1,0,2,3},	// 5

  {4,4,6,1,0,2,5},	// 6

  {5,9,11,1,0,3,4},	// 7	

  {6,10,8,1,0,5,4},	// 8

  {7,8,9,1,0,0,7},	// 9

  {8,11,15,1,0,1,7}, // 10

  {9,14,20,1,0,3,5}, // 11

  {10,6,4,1,0,8,6},	// 12
  {10,2,1,1,0,2,8},	// 13

//------------------------------------------ zde se nic nezpracovava
  {11,20,0,0,0,6,9}, // depo

  {12,14,0,0,0,4,10} // skladka
};

// NASE MESTO
int ulice[POCET_ULIC][7] ={
{1 ,80,1 ,3,0, 0 ,1    },
{1 ,225, 4 ,1,0, 1 ,8    },
{1 ,194, 9 ,2,0, 8 ,14    },
{2 ,110, 8 ,1,0, 1 ,2    },
{2 ,105, 4 ,1,0, 2 ,4    },
{3 ,150, 7 ,3,0, 3 ,2    },
{3 ,230, 9 ,1,0, 2 ,7    },
{4 ,70,4 ,3,0, 4 ,5    },
{5 ,75,5 ,3,0, 5 ,6    },
{6 ,76,5 ,1,0, 8 ,7    },
{6 ,219, 10,3,0, 7 ,9    },
{7 ,128, 5 ,3,0, 9 ,10    },
{7 ,110, 6 ,3,0, 10,11    },
{8 ,211, 10,3,0, 10,12    },
{9 ,90,4 ,3,0, 12,13    },
{10,176, 4 ,1,0, 12,14    },
{11,320, 7 ,1,0, 8 ,15    },
{11,145, 8 ,1,0, 15,33    },
{12,171, 5 ,1,0, 14,15    },
{13,128, 3 ,3,0, 15,16    },
{14,126, 8 ,1,0, 15,17    },
{15,55,2 ,3,0, 17,18    },
{16,100, 4 ,1,0, 18,19    },
{16,112, 6 ,3,0, 19,20    },
{17,158, 6 ,1,0, 1 ,19    },
{18,93,7 ,3,0, 20,21    },
{19,345, 17,1,0, 22,21    },
{19,254, 14,1,0, 21,15    },
{19,110, 10,1,0, 23,33    },
{20,335, 9 ,3,0, 22,0     },
{22,140, 16,3,0, 28,29    },
{23,76,6 ,3,0, 24,25    },
{23,125, 9 ,3,0, 25,26    },
{23,113, 3 ,3,0, 26,30    },
{24,159, 4 ,3,0, 26,27    },
{25,115, 8 ,1,0, 27,28    },
{26,118, 7 ,3,0, 22,29    },
{27,185, 6 ,1,0, 26,31    },
{27,105, 3 ,1,0, 31,32    },
{28,112, 3 ,3,0, 33,34    },
{29,125, 4 ,3,0, 31,34    },
{30,11525, 1 ,0,0, 33,35    },
{31,9423, 1 ,0,0, 34,36    }};



int produkce_odpadu[3][4] = {
    {1,15,4,0},
    {2,90,6,0},
    {3,4,2,0}
};


// Vysledne pole pro nejratsi cestu
int nejkratsi_cesta[POCET_KRIZOVATEK] = {0,0,0,0,0,0,0,0,0};

//  deklarace  globálních  objektů
Facility  Skladka("Skládka odpadu");
Histogram Bagr_doba("Doba Bagrovani odpadu",0,HODINA,30);
Stat Odpad;


class Buldozer: public Process {
    double koeficient;
    double Prichod;
    public: 
        Buldozer () {
            koeficient = 1.0;
        }   

	void Behavior () {
        Prichod = Time;
		while(odpad_ke_zpracovani>0) 
        {
            Bagr_doba(Time); 
            printf("\rBuldozer: ZPracovam odpad\n");
            if (odpad_ke_zpracovani > BULDOZER_KOLIK_ODPADU_ZPRACUJE_ZA_CAS) {
                Wait(BULDOZER_RYCHLOST_ZPRACOVANI_TUNY_ODPADU);
                odpad_ke_zpracovani = odpad_ke_zpracovani - BULDOZER_KOLIK_ODPADU_ZPRACUJE_ZA_CAS;
                buldozer_aktivni = 1;
            } else {
                koeficient = odpad_ke_zpracovani/BULDOZER_RYCHLOST_ZPRACOVANI_TUNY_ODPADU;
                Wait(BULDOZER_RYCHLOST_ZPRACOVANI_TUNY_ODPADU*koeficient);
                odpad_ke_zpracovani = 0;
                buldozer_aktivni = 0;
                printf("\n===================Buldozer: HOTOVO\n");
            }           
            Bagr_doba(Time-Prichod); 
		}
	}
};

class Produkce_odpadu: public Process {
    void Behavior () {
        Wait(7*DEN);
        for (int i = 0; i < 3; ++i)
        {
            produkce_odpadu[i][3] +=  produkce_odpadu[i][1] * PRUMER_MNOZSTVI_ODPADU_CLOVEK;
        }
        //printf("\n PRODUKCE BYTOVKY: %f\n", (double)produkce_odpadu[0][3]);
    }
};



class Auto : public Process {
	double Prichod;                 // atribut každého Auta
	int ujeta_vzdalenost;           // atribut každého Auta
	int trasy_auto[5][MAX_POCET_ULIC_DEN];
	int aktualni_pozice;
	int cilova_pozice; // cilova_pozice
    int mnozstvi_odpadu_v_aute; // kolik kg odpadu popelarsky vuz uveze
    int seznam_ulic_hotov; //  
    int seznam_ulic_pro_dany_den; //  
    int znacka;

	public: 
		Auto (int trasy[5][MAX_POCET_ULIC_DEN]) {
			// ujeta vzdalenost v metrech
			ujeta_vzdalenost = 0;
			aktualni_pozice = DEPO;
			cilova_pozice = -1;
			mnozstvi_odpadu_v_aute = 0; // kdyz vyjizdi z depa tak je prazdny
            seznam_ulic_hotov = 0;
            seznam_ulic_pro_dany_den = -1;

			// trasy ke zpracovani pro cely tyden
			for (int i = 0; i < 5; ++i){
				for (int j = 0; j < MAX_POCET_ULIC_DEN; ++j)
				{
					trasy_auto[i][j] = trasy[i][j];
				}
			}
		}


	void Behavior() {                 // popis chování auta
		
		while(1) {
            zacatek_prace:
			Prichod = Time;               // čas vyjeti auta z depa
            znacka = (rand()%5)+1;        // Oznacim si auto pri vstupu do systemu
			// ja pracovni tyden
			// overeni stavu , pokud je pracovni den a zaroven jeste nema auto hotov svuj prideleny seznam ulic
			if (!je_vikend) 
			{
				// pokud jsem tady, mam novy seznam ulic co mam zpracovat novy den
				int zpracovano_ulic = 0;
				int start_end_nodes[2] = {-1,-1};	// urcuje zacatek a konec ulice
                seznam_ulic_pro_dany_den = den_v_tydnu; // dostal jsem seznam ulic, ktere mam dany den zvladnout

				// zpracovani ulic pro dany den
				while(zpracovano_ulic < MAX_POCET_ULIC_DEN and trasy_auto[den_v_tydnu][zpracovano_ulic]!=-1) 
				{
					// po definovani koncovejch bodu ulice musim resetovat pole aby alg fungoval spravne
					start_end_nodes[0] = -1;
					start_end_nodes[1] = -1;
					if (DEBUG_MODE) printf("\n ============ jsem zde v case %f ====================================\n", Time);
					if (DEBUG_MODE) printf("\n============ zpracovavam ulici %d ============\n", trasy_auto[den_v_tydnu][zpracovano_ulic]);
					
					// podle dne v tydnu zpracovavam ulice
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
					if (DEBUG_MODE) p(start_end_nodes,2);
					if (DEBUG_MODE) printf("\n 1:Moje aktualni pozice: %d \n", aktualni_pozice);

					// zvolim si jako zacatek prvni parametr -> tady bych modl dat i nahodny vyber
					// jakoze ridic si voli trasu
					// sem by se dal dat ((random() < 0.50) ? 0 : 1)
					// mohli by jsme seka dat, ze se ridic rozhoduje bud podle pravdepodobnosti VS vzdalenosti
					cilova_pozice = start_end_nodes[0];		
					int konec_ulice = start_end_nodes[1];

					// presunu se na misto urceni, kde zacu zpracovavat ulici
					// aktualni_pozice je zde porad to nase depo!
					presun(aktualni_pozice,cilova_pozice,&ujeta_vzdalenost);

					// jsem na zacatku ulice
					aktualni_pozice = cilova_pozice;

					///======================== ZPRACOVANÍ ULICE ====================================================


					// jsem v ulici, kde mam zacit delat  dum po domu
					// zde bych mel zacit zpracovavat ulici dum po domu
					// zde zpracovama jednotlive casti ulice

					int zpracovane_useky[MAX_POCET_KRIZOVATEK_ULICE] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
					int zpracovane_krizovatky[MAX_POCET_KRIZOVATEK_ULICE] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
					int zpracovane_useky_index = 0;
					int zpracovane_krizovatky_index = 0;
					if (DEBUG_MODE) printf("\nJsem aktualne v:  %d\n\n", aktualni_pozice);
					for (int i = 0; i < POCET_ULIC; ++i)
					{
						
						// zjstim zdali jsem v te ulici resp, jestli jsem na tom useku kde mam
						// a zdali usek jeste neni zpracovan
						if (ulice[i][0] == id_ulice) 
						{
							if (DEBUG_MODE) printf("\n----------------------------- Ulice: %d Usek: %d\n", ulice[i][0], i);

							int end_point = get_street_by_endpoint(ulice, id_ulice, aktualni_pozice, zpracovane_useky, &zpracovane_useky_index);
							cilova_pozice = get_next_point(ulice,id_ulice,aktualni_pozice, zpracovane_krizovatky,&zpracovane_krizovatky_index);

							if (DEBUG_MODE) printf(" jsem v : %d a Next point: %d Usek: co tam byl: %d index co tam ma byt: %d ep: %d\n",aktualni_pozice, cilova_pozice,i,get_street(ulice,aktualni_pozice,cilova_pozice),end_point);

							int pocet_domu_ulice = ulice[end_point][ULICE_POCET_DOMU];
							int delka_useku_ulice = ulice[end_point][ULICE_DELKA];
							int typ_zastavby = ulice[end_point][ULICE_TYP_ZASTAVBY]; // pocet popelnice

							// musim overit, jestli auto neni plne
							// pokud ne, tak zpracovavam
							int i_zprac = pocet_domu_ulice;

							// pokud je v aute misto, tak zacnu zpracovavat dum po domu
							zpracovavava_ulice:
							if (mnozstvi_odpadu_v_aute<MAX_KAPACITA_ODPADU_V_AUTE) 
							{
								
								while (i_zprac>0) 
								{
									int doba_prijezdu_k_domu = (delka_useku_ulice/pocet_domu_ulice)/PRUMERNA_RYCHLOST_PRESUNU_MEZI_DOMY;
									Wait(doba_prijezdu_k_domu);
									ujeta_vzdalenost+=(delka_useku_ulice/pocet_domu_ulice); // auto se pohlo

									// zpracovani popelnice  v prumeru 30s/popelnice
									Wait(DOBRA_ZPRACOVANI_POPELNICE*produkce_odpadu[typ_zastavby][2]);
									mnozstvi_odpadu_v_aute+= produkce_odpadu[typ_zastavby][3]; // typ zastavby zastupuje pocet popelnic pro danou zastavbu * 50kg na kazdou popelnici
                                    

									// potreba aktualizovat pocet kg odpadu v aute
									// [ sem doplnit pocet odpadu v aute ]
									// Pokud by presahla, tak musime odpad vyvest
									i_zprac--;
								}
							}
							else {
								Odpad(mnozstvi_odpadu_v_aute);
                                svezeny_odpad+=mnozstvi_odpadu_v_aute;

								if (DEBUG_MODE) printf("\nVyvazim odpad: %d\n",mnozstvi_odpadu_v_aute);
								// musim jet s odpadem na skladku
								presun(aktualni_pozice,SKLADKA,&ujeta_vzdalenost);
								// dojel jsem na skladku
								Seize(Skladka);
								Wait(Exponential(DOBA_VYKLAPENI_ODPADU)); // VYKLAPIM OBSAH
								Release(Skladka);

								odpad_ke_zpracovani+=mnozstvi_odpadu_v_aute;
                                if (!buldozer_aktivni) 
                                    (new Buldozer())->Activate();
								mnozstvi_odpadu_v_aute = 0;

								//vracim se ze skladky zpet do te ulice, kterou jsem mel zpracovavat
								presun(SKLADKA,aktualni_pozice,&ujeta_vzdalenost);

								goto zpracovavava_ulice;
							}


							// oznacim ulici jako zpracovanou 
							aktualni_pozice = cilova_pozice;
						}
					}


					///======================== ZPRACOVANÍ ULICE ====================================================
                    printf("\rZN: %d ULICE %d JE HOTOVA,KRIZOVATKA: %d den: %d cas Zpracovani: %f h ujeta_vzdalenost:  %d m cas: %f\n",znacka, trasy_auto[den_v_tydnu][zpracovano_ulic], aktualni_pozice, den_v_tydnu, (Time-Prichod)/3600, ujeta_vzdalenost,Time/3600 );

        			// jakmile dojedu na konec a zpracuju celou ulici, tak nastavim aktualni pozici
					aktualni_pozice = konec_ulice;
					zpracovano_ulic++;
				}

				// V TOMTO miste mam zpracovany ulice
				// mel bych se vratit zpet do depa
               
                if (DEBUG_MODE)printf("\rZN: %d OBLAST JE HOTOVA, jsem na KRIZOVATCE: %d aktualni den: %d cas ztraveny Zpracovanim: %f h ujeta_vzdalenost:  %d m\n",znacka, aktualni_pozice, den_v_tydnu, (Time-Prichod)/3600, ujeta_vzdalenost );
                //ujeta_vzdalenost = 0;
                seznam_ulic_hotov = 1;
                if (DEBUG_MODE)printf("\r--------------------------------------------------------------------------------\n");

                // jsem ve stavu, kdy mame hotovo a musime jet vyklopit odpad a pak jet do depa
                // musim jet s odpadem na skladku
                presun(aktualni_pozice,SKLADKA,&ujeta_vzdalenost);
                // dojel jsem na skladku a potrebuju jet dovnitr abych mohl vyklopit obsah
                Seize(Skladka);
                Wait(DOBA_VYKLAPENI_ODPADU*MINUTA); // VYKLAPIM OBSAH
                Release(Skladka);

                odpad_ke_zpracovani+=mnozstvi_odpadu_v_aute;
                if (!buldozer_aktivni) {
                    (new Buldozer())->Activate();
                }
                mnozstvi_odpadu_v_aute = 0;

                // jedu zpet do depa
                presun(SKLADKA,DEPO,&ujeta_vzdalenost);
                celkova_ujeta_vzdalenost_auta+=ujeta_vzdalenost;
                aktualni_pozice = DEPO;

                // POCKAM DO dalsiho dne
                //Wait(DEN-(Time-Prichod));
                seznam_ulic_hotov = 0;
                //printf("jedu do DEPA cas: %f den: %d\n", Time/3600, den_v_tydnu);

                // proces je v depu i po cely vikend!! nikam nejede, proto ho nevytahhuju o vikendu ź fronty, ale necham ho cekat
                if (den_v_tydnu!=5 && den_v_tydnu!=6) {
                    // proces byl znovu aktivovan ALE ted zahajim cekani, presunu se z parkoviste do garaze, kde pockam, dokud nezacne pracovni tyden
                    Q1.Insert(this);
                    Passivate();
                    //printf("Jedu z depaa:: Je cas vyjet!!\n");
                }

                //goto zacatek_prace;
			}
			else {
                 //printf("GARAZ: cas: %f den: %d\n", Time/3600, den_v_tydnu);
                // zacal vikend, pockam 
                //Wait(2*DEN);
                // cekam v depu,

                WaitUntil(je_vikend==0);
                //printf("KONEC ČEKANI, Vyjizdim Z GARAZE: cas: %f den: %d\n", Time/3600, den_v_tydnu);
				// nevim jeste, jestli to vyuziju
			}
			
		} // end while

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
					//printf("Doslo k zacykleni!!!\n");
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

  // representation
  int dijkstra(int graph[POCET_KRIZOVATEK][POCET_KRIZOVATEK], int src, int dest)
  {
		int dist[POCET_KRIZOVATEK];  // The output array. dist[i] will hold
		bool sptSet[POCET_KRIZOVATEK];
		int parent[POCET_KRIZOVATEK];
		   
		for (int i = 0;i<POCET_KRIZOVATEK; i++)
			parent[i] = -1;
		
		for (int i = 0; i < POCET_KRIZOVATEK; i++)
		{
		  parent[0] = -1;
		  dist[i] = INT_MAX;
		  sptSet[i] = false;
		}

		dist[src] = 0;

		for (int count = 0; count < POCET_KRIZOVATEK-1; count++)
		{
		  int u = minDistance(dist, sptSet);
		  sptSet[u] = true;

		  for (int v = 0; v < POCET_KRIZOVATEK; v++)
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
	int get_street_by_endpoint(int streets[][7], int id_street, int begin_end_point, int processed[MAX_POCET_KRIZOVATEK_ULICE], int* processed_counter) 
	{
		for (int i = 0; i < POCET_ULIC; ++i)
		{
			if (streets[i][0]==id_street) 
				if ( (streets[i][ULICE_KRIZOVATKA_X] == begin_end_point || streets[i][ULICE_KRIZOVATKA_Y] == begin_end_point) &&  !street_in_array(processed, i)) 
				{
					processed[*processed_counter] = i;
					*processed_counter = *processed_counter + 1;
					return i;
				}
		}
		return -1;
	}

	int street_in_array(int processed[MAX_POCET_KRIZOVATEK_ULICE], int street) 
	{
		for (int k = 0; k < MAX_POCET_KRIZOVATEK_ULICE; ++k)
			if (street == processed[k]) 
				return 1;
		return 0;
	}



  	// vraci dalsi uzel ulice id_street, kterej je spojenej s begin_end_point uzlem
	int get_next_point(int streets[][7], int id_street, int begin_end_point, int processed[10], int* processed_counter) 
	{
		for (int i = 0; i < POCET_ULIC; ++i)
		{
			
			if (streets[i][0] == id_street) 
			{
				//printf("\n +++++++++> %d \n", begin_end_point);
				if ( (streets[i][ULICE_KRIZOVATKA_X] == begin_end_point)/* && streets[i][7]!=1*/ && !street_in_array(processed, i)) {
					//printf("\n ++++++ NASEL +++NEXT: FROM: %d DO: %d\n",begin_end_point, streets[i][ULICE_KRIZOVATKA_Y]);
					processed[*processed_counter] = i;
					*processed_counter = *processed_counter + 1;
					return streets[i][ULICE_KRIZOVATKA_Y];
				}

				else if ((streets[i][ULICE_KRIZOVATKA_Y] == begin_end_point) /* && streets[i][7]!=1*/&& !street_in_array(processed, i)) {
					processed[*processed_counter] = i;
					*processed_counter = *processed_counter + 1;
					//printf("\n ++++++ NASEL +++NEXT: FROM: %d DO: %d\n",begin_end_point, streets[i][ULICE_KRIZOVATKA_X]);
					return streets[i][ULICE_KRIZOVATKA_X];
				}
			}
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
        //Activate(Time+DEN);
	}
};

class Gen_odpad : public Event { 
    void Behavior() {                     
        (new Produkce_odpadu())->Activate();
        Activate(Time+DEN);
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

        // zatim nevim, jestli to vyuziju
        while (!Q1.Empty()) {
            Process *aut = (Process *)Q1.GetFirst();
            aut->Activate();
        }// else printf("FRonta BYLaaaa praznda!\n");

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

  Print("***** MODEL *****\n");
  Init(0,4*TYDEN);              // inicializace experimentu

  // stridani dnu v tydnu
  (new Gen_den)->Activate(); 
  (new Gen_odpad)->Activate(); 

  	// jake trasy pojede auto A
	int trasy_A[5][MAX_POCET_ULIC_DEN] = {
		{3,2,1,4,12,6,13,7,26,9},
		{28,29,27,24,29,16,11,8,14,17},
		{18,2,4,25,19,23,22,21,20,26},
		{18,17,3,4,5,6,23,21,16,9},
		{1,2,13,4,27,24,11,7,8,25}
	};

	// popelarsky vuz A
  (new Generator(trasy_A))->Activate(); // generátor zákazníků, aktivace
//  (new Generator(trasy_A))->Activate(); // generátor zákazníků, aktivace
  //(new Generator(trasy_A))->Activate(); // generátor zákazníků, aktivace
  Run();                     // simulace
 Bagr_doba.Output();
Odpad.Output();
Skladka.Output();
  return 0;
}
