#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 5
#endif

#ifndef DEFAULT_CYCLE_COUNT
#define DEFAULT_CYCLE_COUNT 1
#endif


int main(int argc, char* argv[]) {
  size_t cycle_count = DEFAULT_CYCLE_COUNT;

  if (argc > 1) {
    // command line argument passed with a specified cycle count
    printf("argv[1]=%s\n", argv[1]);
    char* endptr; 
    long new_cycle_count = strtol(argv[1], &endptr, 10);
    if (argv[1] == endptr) {
      perror("strtol");
      exit(1);
    }

    cycle_count = (size_t)new_cycle_count;
  }

}

