#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define BASE 1000000000
#define N 26 // number of variables
#define SHIFT 97 // ascii code for 'a'
#define LEFT 40 // ascii code for '('
#define RIGHT 41 // ascii code for ')'
#define EOL 10 // end of line in ascii code
#define EQUAL 61 // ascii code for '='

// value of variable is encoded in numeral system with basis 10^9
typedef struct Variable {
  int *arr;
  int size; // size of allocated memory for arr (size >= len)
  int len; // number of cells used to encode the number
} Variable;

typedef struct Instruction {
  char ins; // instruction from set {ADD, ADD_LOOP, DEC, INC, JUMP, EQUAL, SKIP}
  int var; // index of variable in array of variables
  int adres;
  /* adres is used to store adres (index in array *P in struct Order) of the next instruction
     the value of adres depends on ins, sometimes adres is empty
     for example for ins JUMP it is the index of beginning of the loop
     for DEC it is index of the end of the loop */
} Instruction;

// instructions
#define ADD 'a' // add to variable V[var] variable V[O.I[adres].var]
// by optimal loop I mean optimal code from the task content
#define ADD_LOOP 'l' // start of optimal loop
#define DEC 'd' // dec variable V[var]
#define INC 'i' // inc variale V[var]
#define JUMP 'j' // jump to O.I[adres]
#define SKIP 's' // clear variable V[O.I[adres].var] (it is used after optimal loop)
#define EQ '=' // print value of variable V[var]

/* Order is used to execute single line of Petlik code
   I is a dynamic array of type Instruction
   size and len work the same way as in struct Variable */

typedef struct Order {
  Instruction *I;
  int size;
  int len;
} Order;

// stack to manage loops in Petlik code
struct TStack {
  int value;
  struct TStack *next;
};

typedef struct TStack Stack;

// auxiliary functions min, max, more
int max(int x, int y) {
  return (x > y) ? x : y;
}

int min(int x, int y) {
  return (x < y) ? x : y;
}

int more(int n) {
  return 2 * n + 1;
}

// auxiliary functions pop and push for struct Stack
void pop(Stack **S) {
  if (S == NULL) return;
  Stack *temp = *S;
  *S = (*S)->next;
  free(temp);
}

Stack* push(Stack *S, int v) {
  Stack *new = (Stack*)malloc(sizeof(Stack));
  new->value = v;
  new->next = S;
  return new;
}

/* zeros fills all positions that are allocated but not used at the moment in variable's arr
   after reallloc memory in function realloc_variable
   in order to prevent potential problems in add function */

void zeros(Variable *V) {
  for (int i = V->len; i < V->size; ++i)
    V->arr[i] = 0;
}

/* realloc memory of variable's arr
   n is usually just size of variable V */

void realloc_variable(Variable *V, int n) {
  V->size = more(n);
  V->arr = (int*)realloc(V->arr, (unsigned int) (V->size) * sizeof(int));
  zeros(V);
}

// realoc memory of I array in Order O
void realloc_order(Order *O) {
  O->size = more(O->size);
  O->I = (Instruction*)realloc(O->I, (unsigned int) (O->size) * sizeof(Instruction));
}

// initialize single variable
void init_variable(Variable *V) {
  V->size = 1;
  V->len = 1;
  V->arr = (int*)malloc((unsigned int) sizeof(int));
  V->arr[0] = 0;
}

/* function to initialize all variables
   it is used only once at the beginning */

Variable* init_variables() {
  Variable *V = (Variable*)malloc(N * sizeof(Variable));
  for (int i = 0; i < N; ++i)
    init_variable(&V[i]);
  return V;
}

/* function to initialize Order
   used only once at the beginning */

Order init_order() {
  Order O;
  O.len = 0;
  O.size = N;
  O.I = (Instruction*)malloc((unsigned int) (O.size) * sizeof(Instruction));
  return O;
}

void clear_order(Order *O) {
  O->len = 0;
}

// print value of variable V
void print(Variable *V) {
  int s = V->len - 1;
  printf("%d", V->arr[s]);
  --s;
  while (s >= 0) {
    printf("%09d", V->arr[s]);
    --s;
  }
  printf("\n");
}

// check whether variable V equals zero
bool equal_zero(Variable *V) {
  return (V->len == 1 && V->arr[0] == 0);
}

// add 1 to variable V
void inc(Variable *V) {
  for (int i = 0; i < V->len; ++i) {
    if (V->arr[i] < BASE - 1) {
      V->arr[i] += 1;
      return;
    }
    V->arr[i] = 0;
  }
  // if we reach the end of the loop it means we need to increase the len of variable
  // for example we get here when the number is 999 999 999
  if (V->len == V->size)
    realloc_variable(V, V->size);
  V->arr[V->len] = 1;
  V->len += 1;
}

// minus 1 from variable V
void dec(Variable *V) {
  if (equal_zero(V)) return;
  int i;
  for (i = 0; i < V->len - 1; ++i) {
    if (V->arr[i] > 0) {
      V->arr[i] -= 1;
      return;
    }
    V->arr[i] = BASE - 1;
  }
  // if we reach the end of the loop it means we need to check first number
  // for example we get here when the number is 4 000 000 000
  if (V->arr[i] > 1) {
    V->arr[i] -= 1;
    return;
  }
  else if (V->arr[i] == 1) {
    V->arr[V->len - 1] = 0;
    V->len = max(V->len - 1, 1); // V->len is always >= 1
  }
}

// to variable V adds variable W
void add(Variable *V, Variable *W) {
  // in order to prevent from adding "garbage" values
  // zeros function is crucial here
  if (W->len > V->size) realloc_variable(V, W->len);
  if (V->len > W->size) realloc_variable(W, V->len);

  if (W->len > V->len) V->len = W->len;
  int rest = 0; // rest equals 0 or 1
  for (int i = 0; i < max(V->len, W->len); ++i) {
    int result = V->arr[i] + W->arr[i] + rest;
    V->arr[i] = (result < BASE) ? result : result - BASE;
    rest = (result >= BASE) ? 1 : 0;
  }
  if (rest > 0) {
    // if necessary increase memory for V
    if (V->len == V->size)
      realloc_variable(V, V->size);
    V->arr[V->len] = rest;
    V->len += 1;
  }
}

// set value of variable V to 0
void clear(Variable *V) {
  realloc_variable(V, 0);
  V->len = 1;
  V->arr[0] = 0;
}

// change ascii code to array index, a -> index = 0, b -> index = 1,..., z -> index = 25
int arr_value(int x) {
  return x - SHIFT;
}

/* checks if loop is the most inner loop and there is no the variable
   from the condition of the loop inside the loop
   for example (abcc) is good loop, (a(bc)) and (abaac) are not good
   it is used to choose optimal strategy to execute Petlik program
   excluded is value (in ascii code) of variable from the condition of the loop
   check is by default true, if there is '(' or excluded value then check becomes false */

void check_inside(bool *check, int excluded) {
  int c = getchar();
  if (c == LEFT || c == excluded) *check = false;
  else if (c != RIGHT && c != EOL && c != EOF) check_inside(check, excluded);
  ungetc(c, stdin);
}

// checks whether the program reach the end of file
bool check_EOF() {
  int c = getchar();
  if (c == EOF) return true;
  else
    ungetc(c, stdin);
  return false;
}

/* reads data from the input and fills Order O
   with instructions how to execute Petlik code */

void load(Order *O) {
  // Stack to store left parentheses
  Stack *S = NULL;
  // add = true if we are in the most inner loop at the moment
  bool add = false;
  // add_adres is the beginning of the most inner loop
  int add_adres;
  int c;
  while ((c = getchar()) != EOL) {
    if (O->len == O->size)
      realloc_order(O);
    if (c == LEFT) {
      int v = getchar(); // variable in condition for Powtarzaj
      S = push(S, O->len); // puts index on the top of the stack
      bool check = true;
      check_inside(&check, v);
      O->I[O->len].var = arr_value(v);
      O->I[O->len].ins = (check) ? ADD_LOOP : DEC;
      // if it is the beginning of the most inner loop add becomes true
      // add_adres becomes present index
      if (check) {
        add = true;
        add_adres = O->len;
      }
    }
    else if (c == RIGHT) {
      if (O->I[S->value].ins == DEC) // if present loop is not optimal
        O->I[O->len].ins = JUMP;
      else {
        O->I[O->len].ins = SKIP;
        add = false;
      }
      O->I[S->value].adres = O->len;
      O->I[O->len].adres = S->value;
      pop(&S); // removes the top of the stack
    }
    else if (SHIFT <= c && c < SHIFT + N) {
      O->I[O->len].var = arr_value(c);
      if (add) { // if we are in optimal loop
        O->I[O->len].ins = ADD;
        O->I[O->len].adres = add_adres;
      }
      else
        O->I[O->len].ins = INC;
    }
    else if (c == EQ) {
      int v = getchar();
      O->I[O->len].ins = EQ;
      O->I[O->len].var = arr_value(v);
    }
    (O->len)++;
  }
}

/* process Order O into working code
   instructions are explained under declaration of struct Instruction */

void interpret(Variable *V, Order *O) {
  int i = 0;
  while (i < O->len) {
    char ins = O->I[i].ins;
    if (ins == EQ) {
      print(&V[O->I[i].var]);
      return;
    }
    else if (ins == ADD_LOOP) {
      i = (equal_zero(&V[O->I[i].var])) ? O->I[i].adres : i + 1;
    }
    else if (ins == ADD) {
      int j = O->I[i].adres;
      add(&V[O->I[i].var], &V[O->I[j].var]);
      ++i;
    }
    else if (ins == DEC) {
      if (equal_zero(&V[O->I[i].var]))
        i = O->I[i].adres;
      else {
        dec(&V[O->I[i].var]);
        ++i;
      }
    }
    else if (ins == INC) {
      inc(&V[O->I[i].var]);
      ++i;
    }
    else if (ins == JUMP) {
      int j = O->I[i].adres;
      i = (equal_zero(&V[O->I[j].var])) ? i + 1 : O->I[i].adres;
    }
    else if (ins == SKIP) {
      int j = O->I[i].adres;
      clear(&V[O->I[j].var]);
      ++i;
    }
  }
}

// executes Petlik program
void petlik(Variable *V, Order *O) {
  bool stop = check_EOF();
  while (!stop) {
    load(O);
    interpret(V,O);
    clear_order(O);
    stop = check_EOF();
  }
}

// free all allocated memory at the end
void free_memory(Variable *V, Order *O) {
  for (int i = 0; i < N; ++i)
    free(V[i].arr);
  free(V);
  free(O->I);
}

int main(void) {
  // array of variables of size 26: V[0] -> a, V[1] -> b, ... , V[25] -> z
  Variable *V = init_variables();
  Order O = init_order();
  petlik(V,&O);
  free_memory(V,&O);
  return 0;
}
