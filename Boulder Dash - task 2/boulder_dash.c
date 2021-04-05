#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// zapis w kodzie ASCII odpowiednio znakow 'a','d','s','w'
#define LEWO 97
#define PRAWO 100
#define DOL 115
#define GORA 119
// zapis w kodzei ASCII nowej linii
#define NOWA_LINIA 10
// oznaczenia pola
#define ROCKFORD '@'
#define PUSTE ' '
#define ZIEMIA '+'
#define SKALA '#'
#define KAMIEN 'O'
#define DIAMENT '$'
#define WYJSCIE 'X'

typedef struct Rockford {
  int x; // numer wiersza, w ktorym znajdujde sie Rockford
  int y; // numer kolumny, w ktorej znajdujde sie Rockford
  bool czy_na_planszy; // czy Rockford jest ciagle na planszy
} Rockford;

typedef struct Plansza {
  int w; // liczba wierszy
  int k; // liczba kolumna
  char **p;
  int ile_diamentow_pozostalo; // liczba diamentow na mapie, ktore pozostaly do zebrania
  Rockford R;
} Plansza;

/* Wczytuje liczbe wierszy oraz liczbe kolumna
   nastepnie alokuje potrzebna pamiec na przechowywanie planszy */

void stworz_plansze(Plansza *P) {
  int w, k;
  if (scanf("%d%d", &w, &k)) { // nie mozna uzywac scanf jako void, gdyz wtedy przy kompilacji z plikiem opcje wyswietla blad
    P->w = w;
    P->k = k;
    P->p = (char**)malloc((long unsigned int) w * sizeof(char*));
    for (int i = 0; i < w; ++i)
      P->p[i] = (char*)malloc((long unsigned int) k * sizeof(char));
  }
}

/* Wczytuje plansze */

void wczytaj_plansze(Plansza *P) {
  int c;
  getchar(); // getchar na pominiecie znaku nowej linii
  for (int i = 0; i < P->w; ++i) {
    for (int j = 0; j < P->k; ++j) {
      c = getchar();
      P->p[i][j] = (char) c;
    }
    getchar(); // getchar na pominiecie znaku nowej linii
  }
}

/* Wyswietla plansze */

void wyswietl_plansze(Plansza *P) {
  printf("%d %d\n", P->w, P->k);
  for (int i = 0; i < P->w; ++i) {
    for (int j = 0; j < P->k; ++j) {
      printf("%c", P->p[i][j]);
    }
    printf("\n");
  }
}

/* Szuka wspolrzednych Rockforda na mapie
   funkcja wywolywana tylko raz na poczatku programu */

void znajdz_rockforda(Plansza *P) {
  P->R.czy_na_planszy = false; // na poczatek ustawiamy ze Rockforda nie ma na mapie
  for(int i = 0; i < P->w; ++i) {
    for (int j = 0; j < P->k; ++j) {
      if (P->p[i][j] == ROCKFORD) {
        P->R.x = i;
        P->R.y = j;
        P->R.czy_na_planszy = true;
      }
    }
  }
}

/* Zlicza ile jest diamentow na mapie
   funkcja wywolywana tylko raz na poczatku programu */

void policz_diamenty(Plansza *P) {
  P->ile_diamentow_pozostalo = 0;
  for (int i = 0; i < P->w; ++i) {
    for (int j = 0; j < P->k; ++j) {
      if (P->p[i][j] == DIAMENT)
        ++(P->ile_diamentow_pozostalo);
    }
  }
}

/* Funkcja pomocnicza do stabilizacji planszy
   parametrami sa numer wiersza, numer kolumny oraz zawartosc pola (dolar lub kamien) */

void przesun(Plansza *P, int wiersz, int kolumna, char pole) {
  int it = wiersz;
  // dopoki kolejne pozycje w danej kolumnie sa puste to przesuwamy it
  while (P->p[it + 1][kolumna] == PUSTE) {
    ++it;
  }
  // jezeli jest jakas wolna przestrzen pod to przesuwamy nasz dolar/kamien
  if (it != wiersz) {
    P->p[it][kolumna] = pole;
    P->p[wiersz][kolumna] = PUSTE;
  }
}

/* Stabilizuje konkretna kolumne w planszy, tzn jezeli sa kamienie lub dolary
   ktore moga spasc to je przesuwa */

void stabilizuj_kolumne(Plansza *P, int kolumna) {
  if (P->w <= 3 || P->k <= 2) return; // wowczas plansza sklada sie z samych skal lub z samych skal i jednego wiersza
  for (int i = (P->w) - 2; i >= 1; --i) {
    if (P->p[i][kolumna] == KAMIEN || P->p[i][kolumna] == DIAMENT)
      przesun(P, i, kolumna, P->p[i][kolumna]);
  }
}

/* Stabilizuje cala plansze poprzez stabilizacje kazdej kolumny po kolei */

void stabilizuj_plansze(Plansza *P) {
  for (int i = 1; i < (P->k) - 1; ++i)
    stabilizuj_kolumne(P, i);
}

/* Funkcje pomocnicze do sprawdzania poprawnosci ruchu
   kierunek_w wyznacza zmiane wiersza w zaleznosci od kierunku
   kierunek_k wyznacza zmiane kolumny w zaleznosci od kierunku */

int kierunek_w(int kierunek) {
  if (kierunek == DOL) return 1;
  else if (kierunek == GORA) return -1;
  return 0;
}

int kierunek_k(int kierunek) {
  if (kierunek == PRAWO) return 1;
  else if (kierunek == LEWO) return -1;
  return 0;
}

/* Zwraca zawartosc pola */

char zawartosc_pola(Plansza *P, int kierunek) {
  return P->p[P->R.x + kierunek_w(kierunek)][P->R.y + kierunek_k(kierunek)];
}

/* Sprawdza czy ruch Rockfordem jest dozwolony
   parametrami sa plansza, x i y oznaczajace polozenie Rockforda
   kierunek jest wartoscia ze zbioru {LEWO, PRAWO, DOL, GORA} */

bool czy_dozwolony(Plansza *P, int x, int y, int kierunek) {
  if (kierunek == LEWO || kierunek == PRAWO) {
    int b = kierunek_k(kierunek);
    if ( (P->p[x][y + b] == SKALA) ||
         (P->p[x][y + b] == KAMIEN && P->p[x][y + 2 * b] != PUSTE) ||
         (P->p[x][y + b] == WYJSCIE && P->ile_diamentow_pozostalo != 0) ) return false;
  }
  else {
    int a = kierunek_w(kierunek);
    if ( (P->p[x + a][y] == SKALA) ||
         (P->p[x + a][y] == KAMIEN) ||
         (P->p[x + a][y] == WYJSCIE && P->ile_diamentow_pozostalo != 0) ) return false;
  }
  return true;
}

/* Wykonuje ruch Rockfordem
   parametry to:
   plansza
   wspolrzedne (x,y) oznaczajace polozenie Rockforda przed wykonaniem ruchu
   pomocnicze parametry a i b do okreslenia kierunku ruchu
   zawartosc pola na ktore ma byc wykonany ruch */

void wykonaj_ruch(Plansza *P, int x, int y, int a, int b, char pole) {
  P->p[x][y] = PUSTE;
  if (pole == WYJSCIE)
    P->R.czy_na_planszy = false;
  else {
    (P->R.x) += a;
    (P->R.y) += b;
    P->p[x + a][y + b] = ROCKFORD;
    if (pole == DIAMENT)
      --(P->ile_diamentow_pozostalo);
    else if (pole == KAMIEN)
      P->p[x][y + 2 * b] = KAMIEN;
  }
}

/* Przeprowadza pelna rozgrywke */

void rozgrywka(Plansza *P) {
  int c;
  while ((c = getchar()) != EOF) {
    if (c == NOWA_LINIA)
      wyswietl_plansze(P);
    else if (P->R.czy_na_planszy) { // jezeli Rockforda nie ma juz na planszy to nie wykonujemy ruchu
      if (czy_dozwolony(P, P->R.x, P->R.y, c)) {
        int x = P->R.x;
        int y = P->R.y;
        int a = kierunek_w(c);
        int b = kierunek_k(c);
        char pole = zawartosc_pola(P, c);
        wykonaj_ruch(P, x, y, a, b, pole);
        // jezeli przesuwamy kamien w lewo lub prawo to musimy ustabilizowac kolumne na ktorej konczy ten kamien
        if (pole == KAMIEN)
          stabilizuj_kolumne(P, y + 2 * b);
        // na koniec stablizujemy kolumne, ktora opuscil Rockford
        stabilizuj_kolumne(P, y);
      }
    }
  }
}

/* Na koniec dzialania programu zwalnia wczesniej zaalakowana pamiec */

void zwolnij_pamiec(Plansza *P) {
  for (int i = 0; i < (P->w); ++i)
    free(P->p[i]);
  free(P->p);
}

int main(void) {
  Plansza P;
  stworz_plansze(&P);
  wczytaj_plansze(&P);
  stabilizuj_plansze(&P);
  wyswietl_plansze(&P);
  znajdz_rockforda(&P);
  policz_diamenty(&P);
  rozgrywka(&P);
  zwolnij_pamiec(&P);
  return 0;
}
