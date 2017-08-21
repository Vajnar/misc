/*
 * Efficient Tabu search algorithm implementation
 * for solving Pj, Dj, Wj problems with respect
 * to minimizing total weighted tardiness
 *
 * by Martin Vajnar, 2013-5
 *
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>

#define ARRAY_SIZE(x)  sizeof(x)/sizeof(x[0])

#define TABU_LENGTH  11
#define ITERATIONS   200

typedef struct {
  int pj;
  int dj;
  int wj;
  int id;
} task;

static task tasks_initial[] = { {16,  67, 45,  2},
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
  static task* tabu[TABU_LENGTH][2];
#endif

static void iter_printout(task **tasks, int fitness, int iter) {
  task **end;
  char buffer[200], *start = buffer;

  start += sprintf(start, "%3d: ", iter);
  for(end = tasks + ARRAY_SIZE(tasks_initial); tasks < end; tasks++)
    start += sprintf(start, "%2d, ", (*tasks)->id);
  sprintf(start, "(%d)",fitness);

  puts(buffer);
}

static void printout(task **tasks_ref, int fitness) {
  task **end;
  char buffer[500], *start = buffer;

  start += sprintf(start, "Initial schedule (Id: Pj, Dj, Wj):\n");
  for (end = tasks_ref + ARRAY_SIZE(tasks_initial); tasks_ref < end; tasks_ref++)
    start += sprintf(start, "%2d: %2d, %3d, %2d\n",
      (*tasks_ref)->id, (*tasks_ref)->pj, (*tasks_ref)->dj, (*tasks_ref)->wj);
  sprintf(start, "Fitness: %d\n\nIteration step: best schedule, (fitness):", fitness);

  puts(buffer);

  iter_printout(tasks_ref - ARRAY_SIZE(tasks_initial), fitness, 0);
}

static void swap(task **a) {
  task *temp = a[0];

  a[0] = a[1];
  a[1] = temp;
}

static int is_in_tabu(task* restrict a, task* restrict b) {
#if TABU_LENGTH > 0
  for (task* (*temp)[2] = tabu; temp < tabu + ARRAY_SIZE(tabu); temp++) {
    if ((*temp)[0] == a && (*temp)[1] == b)
      return 1;
  }
#endif
  return 0;
}

static void add_to_tabu(task* restrict a, task* restrict b) {
#if TABU_LENGTH > 1
  static task* (*temp)[2] = tabu;

  (*temp)[0] = a;
  (*temp)[1] = b;

  if (temp == tabu + ARRAY_SIZE(tabu) - 1)
    temp = tabu;
  else
    temp++;
#elif TABU_LENGTH == 1
  tabu[0][0] = a;
  tabu[0][1] = b;
#endif
}

static unsigned int compute_initial_fitness(void) {
  task *temp;
  unsigned int time, fitness;

  for (temp = tasks_initial, time = temp->pj, fitness = 0;
       temp < tasks_initial + ARRAY_SIZE(tasks_initial);
       time += (++temp)->pj) {
    int tj;

    if ((tj = time - temp->dj) > 0)
      fitness += tj * temp->wj;
  }
  return fitness;
}

static unsigned int compute_fitness(task* restrict a, task* restrict b, unsigned int fitness_prev_it, unsigned int time) {
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
  unsigned int i, fitness_best, fitness_prev_it, pass = 0;
  task *tasks_best[ARRAY_SIZE(tasks_initial)], *tasks_prev_it[ARRAY_SIZE(tasks_initial)];

  _Static_assert(TABU_LENGTH <
                  (ARRAY_SIZE(tasks_initial) * (ARRAY_SIZE(tasks_initial) - 1)) / 2,
                "Tabu list is too long.");

  fitness_best = fitness_prev_it = compute_initial_fitness();
  for (i = 0; i < ARRAY_SIZE(tasks_initial); i++)
    tasks_best[i] = tasks_prev_it[i] = &tasks_initial[i];

  printout(tasks_best, fitness_best);

  for (i = 1; i <= ITERATIONS; i++) {
    unsigned int time, fitness_curr_it;
    task **tasks_temp, **perm;

    for (time = 0, tasks_temp = tasks_prev_it, fitness_curr_it = UINT_MAX;
         tasks_temp < tasks_prev_it + ARRAY_SIZE(tasks_initial) - 1;
         time += (*tasks_temp++)->pj) {
      if (!is_in_tabu(tasks_temp[0], tasks_temp[1])) {
        unsigned int fitness_temp = compute_fitness(tasks_temp[0], tasks_temp[1], fitness_prev_it, time);
        if (fitness_temp < fitness_curr_it) {
          fitness_curr_it = fitness_temp;
          perm = tasks_temp;
        }
      }
    }

    swap(perm);
    add_to_tabu(perm[0], perm[1]);

    fitness_prev_it = fitness_curr_it;
    if (fitness_curr_it < fitness_best) {
      fitness_best = fitness_curr_it;
      memcpy(tasks_best, tasks_prev_it, sizeof(tasks_best));
      pass = i;
    }

    iter_printout(tasks_prev_it, fitness_curr_it, i);
  }

  puts("\nBest schedule:");
  iter_printout(tasks_best, fitness_best, pass);

  return 0;
}
