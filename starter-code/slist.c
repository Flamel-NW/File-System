// based on cs3650 starter code

#include <alloca.h>
#include <stdlib.h>
#include <string.h>

#include "slist.h"

slist_t *s_cons(const char *text, slist_t *rest) {
  slist_t *xs = malloc(sizeof(slist_t));
  xs->data = strdup(text);
  xs->refs = 1;
  xs->next = rest;
  return xs;
}

void s_free(slist_t *xs) {
  if (xs == 0) {
    return;
  }

  xs->refs -= 1;

  if (xs->refs == 0) {
    s_free(xs->next);
    free(xs->data);
    free(xs);
  }
}

slist_t *s_explode(const char *text, char delim) {
  if (*text == 0) {
    return 0;
  }

  int plen = 0;
  while (text[plen] != 0 && text[plen] != delim) {
    plen += 1;
  }

  int skip = 0;
  if (text[plen] == delim) {
    skip = 1;
  }

  slist_t *rest = s_explode(text + plen + skip, delim);
  char *part = alloca(plen + 2);
  memcpy(part, text, plen);
  part[plen] = 0;

  return s_cons(part, rest);
}
