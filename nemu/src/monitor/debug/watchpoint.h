#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <common.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  word_t value;
  char expr[32];
} WP;

word_t expr(char *e , bool *success);
WP* new_wp(char *exp);
void free_wp(WP *wp);
bool is_changed();
void delete_wp(int NO);
void print_wp();

#endif
