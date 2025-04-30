#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 5 // >62 overflows ASCII table. <2 seg faults
#endif

#ifndef DEFAULT_CYCLE_COUNT
#define DEFAULT_CYCLE_COUNT 1
#endif

// Size of a philosopher column independent of fork amount
#ifndef PHILOSOPHER_PRINT_SPACE_BASE
#define PHILOSOPHER_PRINT_SPACE_BASE 9
#endif

static size_t cycle_count = DEFAULT_CYCLE_COUNT;
static char *status_line = NULL; // Null to start

void dawdle();
void print_header();
void print_status(size_t start, char *str);

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
    char *endptr;
    long new_cycle_count = strtol(argv[1], &endptr, 10);
    if (argv[1] == endptr) {
      perror("strtol");
      exit(EXIT_FAILURE);
    }

    cycle_count = (size_t)new_cycle_count;
  }

  if (NUM_PHILOSOPHERS < 2) {
    printf("At least 2 philosophers required.\n");
    exit(EXIT_FAILURE);
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

  // Print out the header
  print_header();

  // Spawn philosopher threads
  pthread_t thread_id[NUM_PHILOSOPHERS]; // Where to store thread ids
  for (i = 0; i < NUM_PHILOSOPHERS; i++) {

    // Creating the data a philosopher will need
    phil_arg *phil_stuff = (phil_arg *)malloc(sizeof(phil_arg));
    if (!phil_stuff) {
      perror("malloc: phil_stuff");
      exit(EXIT_FAILURE);
    }
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
  free(status_line);

  return 0;
}

// Prints out the philosopher header
// Stretches according to the number of forks
void print_header() {
  int i;
  int j;

  // The margin in between two philosopher columns
  // PHILOSOPHER_PRINT_SPACE_BASE include the | at the end,
  // so remove it (-1).
  int margin_space = PHILOSOPHER_PRINT_SPACE_BASE + NUM_PHILOSOPHERS - 1;

  // Top line of header
  printf("|");
  for (i = 0; i < NUM_PHILOSOPHERS; i++) {
    for (j = 0; j < margin_space; j++) {
      printf("=");
    }
    printf("|");
  }

  // Middle line of header
  printf("\n|");
  for (i = 0; i < NUM_PHILOSOPHERS; i++) {
    if (margin_space % 2 == 0) {
      for (j = 0; j < (margin_space / 2) - 1; j++) {
        printf(" ");
      }
      printf("%c", 'A' + i);
      for (j = 0; j < (margin_space / 2); j++) {
        printf(" ");
      }
      printf("|");
    } else {
      for (j = 0; j < (margin_space / 2); j++) {
        printf(" ");
      }
      printf("%c", 'A' + i);
      for (j = 0; j < (margin_space / 2); j++) {
        printf(" ");
      }
      printf("|");
    }
  }

  // Bottom line of header
  printf("\n|");
  for (i = 0; i < NUM_PHILOSOPHERS; i++) {
    for (j = 0; j < margin_space; j++) {
      printf("=");
    }
    printf("|");
  }
  printf("\n");

  // Initialize the status line.
  print_status(0, NULL);
}

// Prints out the status line
// Must be initialized first to allocate memory for the status line.
// Free'd from main
// start: where to modify from
// str: what to modify to

// status_line[2:NUM_PHILOSOPHERS + 1]
// ^^^ Part of status line with forks (inclusive)

// status_line[NUM_PHILOSOPHERS + 4:NUM_PHILOSOPHERS + 8]
// ^^^ Eat/Think part (inclusive)
void print_status(size_t start, char *str) {
  int i;
  int j;

  // Edge case: Printing initial status line with no change in state
  if (status_line == NULL) {
    size_t status_line_size =
        (PHILOSOPHER_PRINT_SPACE_BASE + NUM_PHILOSOPHERS) * NUM_PHILOSOPHERS + 3; // +3 for null terminator, newline, and final |
    status_line = (char *)malloc(sizeof(char) * status_line_size);
    if (!status_line) {
      perror("malloc: status_line");
      exit(EXIT_FAILURE);
    }

    size_t margin_space = PHILOSOPHER_PRINT_SPACE_BASE + NUM_PHILOSOPHERS;
    // Set the status_line to be blank
    // i * margin_space is the column offset
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
      size_t base = i * margin_space;

      // Printing start of column
      status_line[0 + base] = '|';
      status_line[1 + base] = ' ';

      // Printing for fork placeholders
      for (j = 2; j < NUM_PHILOSOPHERS + 2; j++) {
        status_line[j + base] = '-';
      }
      
      // 7 spaces after fork placeholders
      // 2 is past the '| ' at the start of the column
      int space_count = PHILOSOPHER_PRINT_SPACE_BASE - 2;
      for (j = 0; j < space_count; j++) {
        status_line[j + NUM_PHILOSOPHERS + base + 2] = ' ';
      }
    }
    // Pipe and newline at the end
    status_line[status_line_size - 3] = '|';
    status_line[status_line_size - 2] = '\n';
    status_line[status_line_size - 1] = '\0';
  }
  printf("%s", status_line);
}
