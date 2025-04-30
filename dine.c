#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 62 // >62 overflows ASCII table
#endif

#ifndef DEFAULT_CYCLE_COUNT
#define DEFAULT_CYCLE_COUNT 1
#endif

static size_t cycle_count = DEFAULT_CYCLE_COUNT;

void dawdle();

// Every philosopher needs to see its forks
// And know its name
typedef struct phil_arg {
  char name;
  sem_t **forks;
} phil_arg;

void *philosopher(void *arg) {
  int i;

  // Unpacking the argument
  phil_arg *phil_stuff = (phil_arg *)arg;
  char name = phil_stuff->name;
  sem_t **forks = phil_stuff->forks;

  // Getting our forks
  size_t leftfork_idx = name - 65;

  // Handles case where the last philosophers right fork is the 1st
  // philosophers left fork.
  size_t rightfork_idx;
  if (leftfork_idx == NUM_PHILOSOPHERS - 1) {
    rightfork_idx = 0;
  } else {
    rightfork_idx = leftfork_idx + 1;
  }

  sem_t *leftfork = forks[leftfork_idx];
  sem_t *rightfork = forks[rightfork_idx];

  printf("Philosopher %c entered.\n", name);

  // States:
  // No forks, trying to acquire fork
  // 1 fork, trying to acquire other fork
  // Both forks acquired, trying to eat
  // Done eating, both forks still in hand
  // Released 1 fork, going to release other
  // Released both forks,

  for (i = 0; i < cycle_count; i++) {
    // Even philosophers first pickup right fork
    // Odd philosophers first pickup left fork
    if (name % 2 == 0) {
      sem_wait(rightfork);
      // TODO: Print 1st fork acquired
      sem_wait(leftfork);
      // TODO: Print 2nd fork acquired

      // TODO: Print eating with all forks
      dawdle();
      // TODO: Print done eating (just holding both forks)

      sem_post(leftfork);
      // TODO: Print returned the 2nd fork

      sem_post(rightfork);
      // TODO: Print returned the 1st fork

      // TODO: Print thinking
      dawdle();
      // TODO: Print done thinking (doing nothing)

    } else {
      sem_wait(leftfork);
      // TODO: Print 1st fork acquired
      sem_wait(rightfork);
      // TODO: Print 2nd fork acquired

      // TODO: Print eating with all forks
      dawdle();
      // TODO: Print done eating (just holding both forks)

      sem_post(rightfork);
      // TODO: Print returned the 2nd fork

      sem_post(leftfork);
      // TODO: Print returned the 1st fork

      // TODO: Print thinking
      dawdle();
      // TODO: Print done thinking (doing nothing)
    }
  }

  // Philosopher has to release their stuff.
  free(phil_stuff);

  printf("Philosopher %c exited.\n", name);
  return NULL;
}

int main(int argc, char *argv[]) {
  int i;

  if (argc > 1) {
    // command line argument passed with a specified cycle count
    printf("argv[1]=%s\n", argv[1]);
    char *endptr;
    long new_cycle_count = strtol(argv[1], &endptr, 10);
    if (argv[1] == endptr) {
      perror("strtol");
      exit(EXIT_FAILURE);
    }

    cycle_count = (size_t)new_cycle_count;
  }

  // Allocate forks (semaphores) for philosophers on heap
  sem_t **forks = (sem_t **)malloc(sizeof(sem_t *) * NUM_PHILOSOPHERS);
  if (!forks) {
    perror("malloc forks list");
    exit(EXIT_FAILURE);
  }

  // Allocate individial semaphores
  for (i = 0; i < NUM_PHILOSOPHERS; i++) {
    sem_t *address = (sem_t *)malloc(sizeof(sem_t));
    if (!address) {
      perror("malloc fork");
      exit(EXIT_FAILURE);
    }
    forks[i] = address;
  }

  for (i = 0; i < NUM_PHILOSOPHERS; i++) {
    // 2nd argument indicates using threads
    // 3rd argument is initial value (all forks are available)
    int res = sem_init(forks[i], 0, 1);
    if (res) {
      perror("sem_init");
      exit(EXIT_FAILURE);
    }
  }

  // Semaphore for printing authority
  sem_t *print_sem = (sem_t *)malloc(sizeof(sem_t));
  if (!print_sem) {
    perror("malloc print_sem");
    exit(EXIT_FAILURE);
  }
  int res = sem_init(print_sem, 0, 1);
  if (res) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  // Spawn philosopher threads
  pthread_t thread_id[NUM_PHILOSOPHERS]; // Where to store thread ids
  for (i = 0; i < NUM_PHILOSOPHERS; i++) {

    // Creating the data a philosopher will need
    phil_arg *phil_stuff = (phil_arg *)malloc(sizeof(phil_arg));
    phil_stuff->name = 'A' + i;
    phil_stuff->forks = forks;

    res = pthread_create(thread_id + i, NULL, philosopher, (void *)phil_stuff);
  }

  // Let the philosophers finish
  for (i = 0; i < NUM_PHILOSOPHERS; i++) {
    pthread_join(thread_id[i], NULL);
  }

  // Free all the semaphores from memory
  for (i = 0; i < NUM_PHILOSOPHERS; i++) {
    sem_destroy(forks[i]);
    free(forks[i]);
  }
  sem_destroy(print_sem);
  free(print_sem);
  free(forks);

  return 0;
}
