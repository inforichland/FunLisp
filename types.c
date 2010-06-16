#include <stdio.h>
#include <stdlib.h>

void die(const char *msg) {
  fprintf(stderr, msg);
  exit(1);
}
