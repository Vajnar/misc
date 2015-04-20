/*
 * Efficient Tabu search algorithm implementation
 * for solving Pj, Dj, Wj problems with respect
 * to minimizing total tardiness
 *
 * by Martin Vajnar, 2013
 *
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>

#define ITEMS        15
#define TABU_LENGTH  11
#define NUM_CYCLES   200

#if TABU_LENGTH >= (ITEMS * (ITEMS - 1)) / 2
  #error "Tabu list is too long."
#endif

typedef struct {
  int pj;
  int dj;
  int wj;
  int id;
} task;

static task tasks_initial[ITEMS] = { {16,  67, 45,  2},
                                     { 6, 105, 35,  3},
                                     {12,   8, 80, 15},
                                     {19, 124, 28,  6},
                                     { 9,  77,  1,  5},
                                     {20, 202, 70, 10},
                                     {13, 157, 14,  8},
                                     { 1, 194, 21,  7},
                                     { 5,   5, 69, 13},
                                     {18,   7, 62, 14},
                                     { 4,  36, 21,  1},
                                     { 5,  53, 73,  4},
                                     {19,  61, 23, 12},
                                     {12,  25, 76,  9},
                                     {20,  43, 51, 11} };
#if TABU_LENGTH > 0
  static task* tabu[TABU_LENGTH*2];
#endif

static void iter_printout(task **tasks, int fitness, int iter) {
  task **end;
  char buffer[200], *start = buffer;

  start += sprintf(start,"%3d: ",iter);
  for(end=tasks+ITEMS;tasks<end;tasks++)
    start += sprintf(start,"%2d, ",(*tasks)->id);
  sprintf(start,"(%d)",fitness);

  puts(buffer);
}

static void printout(task **tasks_ref, int fitness) {
  task **ende;
  char buffer[500], *start = buffer;

  start += sprintf(start,"Initial schedule (Id: Pj, Dj, Wj):\n");
  for (ende = tasks_ref+ITEMS; tasks_ref<ende; tasks_ref++)
    start += sprintf(start,"%2d: %2d, %3d, %2d\n",
      (*tasks_ref)->id,(*tasks_ref)->pj,(*tasks_ref)->dj,(*tasks_ref)->wj);
  sprintf(start,"Fitness: %d\n\nIteration step: best schedule, (fitness):",fitness);
  puts(buffer);

  iter_printout(tasks_ref-ITEMS,fitness,0);
}

static void swap(task **a) {
  task *temp = *a;

  *a = *(a+1);
  *(a+1) = temp;
}

static int is_in_tabu(task *a, task *b) {
#if TABU_LENGTH > 0
  task **temp;

  for (temp=tabu; temp<tabu+TABU_LENGTH*2; temp+=2)
    if (*temp == a && *(temp+1) == b)
      return 1;
#endif
  return 0;
}

static void add_to_tabu(task *a, task *b) {
#if TABU_LENGTH > 1
  static task **temp = tabu;

  *temp = a; *(temp+1) = b;

  if (temp == tabu+(TABU_LENGTH-1)*2) temp = tabu;
  else temp+=2;
#elif TABU_LENGTH == 1
  *tabu = a; *(tabu+1) = b;
#endif
}

static int compute_initial_fitness(void) {
  task *temp;
  int time=0, fitness=0;

  for (temp = tasks_initial; temp<tasks_initial+ITEMS; temp++) {
    int tj;

    time += temp->pj;
    if ((tj = time - temp->dj) > 0)
      fitness += tj * temp->wj;
  }
  return fitness;
}

static int compute_fitness(task *a, task *b, int fitness_prev_it, int time) {
  int tj;

  time += a->pj;
  if((tj = time - a->dj) > 0)
    fitness_prev_it -= tj * a->wj;

  time += b->pj;
  if((tj = time - b->dj) > 0)
    fitness_prev_it -= tj * b->wj;

  time -= a->pj;
  if((tj = time - b->dj) > 0)
    fitness_prev_it += tj * b->wj;

  time += a->pj;
  if((tj = time - a->dj) > 0)
    fitness_prev_it += tj * a->wj;

  return fitness_prev_it;
}

int main(void) {
  int i, fitness_best, fitness_prev_it, pass = 0;
  task *tasks_best[ITEMS], *tasks_prev_it[ITEMS];

  for (i=0; i<ITEMS; i++) tasks_best[i] = tasks_prev_it[i] = tasks_initial+i;

  fitness_best = fitness_prev_it = compute_initial_fitness();
  printout(tasks_best,fitness_best);

  for (i=1; i<=NUM_CYCLES; i++) {
    int time = 0, fitness_curr_it = INT_MAX;
    task **tasks_temp, **perm;

    for (tasks_temp=tasks_prev_it; tasks_temp<tasks_prev_it+ITEMS-1;
         time+=(*tasks_temp++)->pj) {
      int fitness_temp;

      if (is_in_tabu(*tasks_temp,*(tasks_temp+1)))
        continue;

      fitness_temp = compute_fitness(*tasks_temp,*(tasks_temp+1),fitness_prev_it,time);

      if (fitness_temp < fitness_curr_it) {
        fitness_curr_it = fitness_temp;
        perm = tasks_temp;
      }
    }

    swap(perm);
    add_to_tabu(*perm,*(perm+1));
    fitness_prev_it = fitness_curr_it;

    if (fitness_curr_it < fitness_best) {
      fitness_best = fitness_curr_it;
      memcpy(tasks_best,tasks_prev_it,sizeof(tasks_best));
      pass = i;
    }

    iter_printout(tasks_prev_it,fitness_curr_it,i);
  }

  puts("\nBest schedule:");
  iter_printout(tasks_best,fitness_best,pass);

  return 0;
}
