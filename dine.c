#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 5
#endif

#ifndef DEFAULT_CYCLE_COUNT
#define DEFAULT_CYCLE_COUNT 1
#endif

typedef int semaphore; // A semaphore is just an integer

void dawdle();

void *philosopher(void *arg) {
  char name = *(char *)arg;

  printf("Philosopher %c entered.\n", name);
  dawdle();
  printf("Philosopher %c exited.\n", name);

  return NULL;
}

int main(int argc, char *argv[]) {
  size_t cycle_count = DEFAULT_CYCLE_COUNT;
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
    char name = 'A' + i;
    res = pthread_create(thread_id + i, NULL, philosopher, (void *)&name);
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
}
