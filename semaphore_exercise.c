#include "base.h"
#include <pthread.h>
#include <unistd.h>

/*
 * constants
 */

#define N_THREADS 4

/*
 * tables
 */


/*
 * types
 */

/*
 * struct bodies
 */

/*
 * headers
 */

int main(void);

/*
 * globals
 */

Arena *scratch;

/*
 * functions
 */

int main(void) {
  scratch = arena_create(MB(1));

  Str8 file_text = os_read_entire_file(scratch, str8_lit("hello.txt"));

  printf("parsing file...\n");

  return 0;
}
