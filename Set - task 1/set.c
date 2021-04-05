#include <stdio.h>
#include <stdbool.h>

// rozmiar talii kart
#define TALIA_KART 81

// maksymalna liczba kart na stole
#define MAX_NA_STOLE 21

// liczba kart na stole na poczatku gry
#define ROZMIAR_STOLU 12

// rozmiar seta
#define SET 3

typedef struct Karta {
  int l; // liczba figur
  int kol; // kolor
  int w; // wypelnienie
  int k; // ksztalt
} Karta;

typedef struct Talia {
  int ile_kart_na_poczatku; // ile kart zostalo wczytanych na wejsciu
  int ile_kart_w_grze; // ile kart zostalo do tej pory uzyte w grze
  Karta tab[TALIA_KART]; // wszystkie karty w talii
} Talia;

typedef struct Stol {
  int ile_na_stole; // ile jest kart na stole w danym momencie
  Karta tab[MAX_NA_STOLE]; // karty, ktore sa na stole
} Stol;

// funkcja pomocnicze min
int min(int x, int y) {
  return (x < y) ? x : y;
}

// Funkcja pomocnicza do wczytywania, zamienia znak z kodu ASCI na liczbe 1, 2 lub 3
int zamien_z_ascii(int x) {
  if (x == 49) return 1;
  else if (x == 50) return 2;
  else return 3;
}

/* Wczytywanie danych z pliku
   kazda wczytana karta zapisywana jest do talii kart, ktora
   jest przekazana jako parametr funkcji, przy kazdym
   dodaniu karty zwieksza sie licznik (ile_kart_jak_dotad)
   na koniec ustawiana jest wartosc ile_kart_na_poczatku */

void wczytaj(Talia *T) {
  int ile_kart_jak_dotad = 0;
  int a, b, c, d;
  while ((a = getchar()) != EOF) {
      if (a == 49 || a == 50 || a == 51) {
        b = getchar();
        c = getchar();
        d = getchar();
        Karta pom = {zamien_z_ascii(a), zamien_z_ascii(b), zamien_z_ascii(c), zamien_z_ascii(d)};
        T->tab[ile_kart_jak_dotad] = pom;
        ile_kart_jak_dotad++;
      }
  }
  T->ile_kart_na_poczatku = ile_kart_jak_dotad;
}

/* Wyswietlanie karty
   jako parametry mamy karte oraz wartosc bool oznaczajaca
   czy jest to ostatnia karta do wyswietlenia w wierszu
   jest to istotne, gdyz nie chcemy na koncu pisac spacji,
   tylko przejsc od razu do kolejnego wiersza */

void wyswietl_karte(Karta K, bool ostatnia) {
  if (ostatnia) printf("%d%d%d%d\n", K.l, K.kol, K.w, K.k);
  else printf("%d%d%d%d ", K.l, K.kol, K.w, K.k);
}

/* Wyswietlanie aktualnego stolu
   jezeli stol jest pusty to wyswietla sie tylko =
   w przeciwnym razie wyswietla sie kazda karta po kolei */

void wyswietl_stol(Stol S) {
  int n = S.ile_na_stole;
  if (n == 0) {
    printf("=\n");
    return;
  }
  printf("= ");
  for (int i = 0; i < n; ++i) {
    if (i != n - 1) wyswietl_karte(S.tab[i], 0);
    else wyswietl_karte(S.tab[i], 1);
  }
}

/* Wyswietlanie seta
   parametry to stol S oraz indeksy k_1, k_2 i k_3
   sa to indeksy kolejnych kart, ktore utworzyly seta */

void wyswietl_set(Stol S, int k_1, int k_2, int k_3) {
  printf("- ");
  wyswietl_karte(S.tab[k_1],0);
  wyswietl_karte(S.tab[k_2],0);
  wyswietl_karte(S.tab[k_3],1);
}

/* Po wczytaniu talii kart z pliku, uzupelniamy stol o
   12 kart (lub mniej jezeli tyle nie ma w talii)
   jest to funkcja wykonywana tylko raz na poczatku gry */

void wyloz_karty(Talia *T, Stol *S) {
  for (int i = 0; i < min(T->ile_kart_na_poczatku, ROZMIAR_STOLU); ++i) {
    S->tab[i] = T->tab[i];
    S->ile_na_stole++;
    T->ile_kart_w_grze++;
  }
}

/* Sprawdza czy w talii sa dostepne jeszcze karty
   jezeli tak to doklada 3 karty na stol */

void dodaj_karty(Talia *T, Stol *S) {
  if (T->ile_kart_na_poczatku > T->ile_kart_w_grze) {
    int n = S->ile_na_stole;
    int it = T->ile_kart_w_grze;
    for (int i = n; i < n + SET; ++i) {
      S->tab[i] = T->tab[it];
      S->ile_na_stole++;
      T->ile_kart_w_grze++;
      it++;
    }
  }
}

/* Funkcja pomocnicza do aktualizuj_stol
   przesun uzupelnia "luki" na stole, ktore
   pojawiaja sie po znalezieniu seta
   tzn przesuwa wszystkie karty od konca tak zeby
   zajely miejsca kart, ktore utworzyly seta,
   natomiast na koncu dzieki temu mamy 3 wolne miejsca
   na nowe karty */

void przesun(Stol *S, int k) {
  for (int i = k; i < S->ile_na_stole - 1; ++i)
    S->tab[i] = S->tab[i + 1];
}

/* Po znalezieniu seta aktualizuje stol
   parametrami sa talia, stol oraz 3 indeksy kart
   indeksy odwoluja sie do tablicy w strukturze stol */

void aktualizuj_stol(Talia *T, Stol *S, int k_1, int k_2, int k_3) {
  przesun(S,k_3);
  przesun(S,k_2);
  przesun(S,k_1);
  S->ile_na_stole -= SET;
  if (S->ile_na_stole < ROZMIAR_STOLU)
    dodaj_karty(T,S);
}

/* Dla 3 kart sprawdza czy spelniaja one warunki aby utworzyc set */

bool set(Karta K_1, Karta K_2, Karta K_3) {
  return (((K_1.l == K_2.l && K_2.l == K_3.l) || (K_1.l != K_2.l && K_1.l != K_3.l && K_2.l != K_3.l)) &&
          ((K_1.kol == K_2.kol && K_2.kol == K_3.kol) || (K_1.kol != K_2.kol && K_1.kol != K_3.kol && K_2.kol != K_3.kol)) &&
          ((K_1.w == K_2.w && K_2.w == K_3.w) || (K_1.w != K_2.w && K_1.w != K_3.w && K_2.w != K_3.w)) &&
          ((K_1.k == K_2.k && K_2.k == K_3.k) || (K_1.k != K_2.k && K_1.k != K_3.k && K_2.k != K_3.k)));
}

/* Szuka z kart ktore sa na stole seta,
   jezeli znajdzie to wyswietla karty tworzace set
   oraz aktualizuje stol i zwraca true
   jezeli wszystkie elementy zostana sprawdzone i set
   nie zostanie znaleziony to funkcja zwraca false */

bool szukaj_seta(Talia *T, Stol *S) {
  int n = S->ile_na_stole;
  int l, s, p;
  for (l = 0; l <= n - 3; ++l) {
    for (s = l + 1; s <= n - 2; ++s) {
      for (p = s + 1; p <= n - 1; ++p) {
        if (set(S->tab[l], S->tab[s], S->tab[p])) {
          wyswietl_set(*S, l, s, p);
          aktualizuj_stol(T, S, l, s, p);
          return true;
        }
      }
    }
  }
  return false;
}

/* Funkcja symulujaca cala rozgrywke
   na poczatek tworzy struktury stol oraz talia
   wczytuje do talii karty z pliku
   wyklada karty na stol
   nastepnie przeprowadza pelna rozgrywke
   az nie zostanie osiagniety warunek stopu */

void rozgrywka() {
  Talia T;
  Stol S;
  T.ile_kart_na_poczatku = 0;
  T.ile_kart_w_grze = 0;
  S.ile_na_stole = 0;
  wczytaj(&T);
  wyloz_karty(&T, &S);
  bool graj = true;
  while (graj) {
    wyswietl_stol(S);
    graj = szukaj_seta(&T, &S);
    // jezeli w talii sa karty do dolozenia to kontynuujemy gre
    if(!graj) {
      if (T.ile_kart_na_poczatku > T.ile_kart_w_grze) {
        printf("+\n");
        dodaj_karty(&T,&S);
        graj = true;
      }
    }
  }
}

int main(void) {
    rozgrywka();
    return 0;
}
