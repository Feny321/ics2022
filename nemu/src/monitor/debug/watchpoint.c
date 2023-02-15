#include "watchpoint.h"
#include "expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char *exp){
  if(free_ == NULL) assert(0);
  WP* temp = free_;
  free_ = free_->next;
  temp->next = NULL;
  strcpy(temp->expr , exp);
  bool success = false;
  temp->value = expr(exp , &success);

  if(head == NULL){
    head = temp;
  }else{
    WP *t;
    t = head;
    while(t->next != NULL){
      t = t->next;
    }
    t->next = temp;
  }
  return temp;
}

void free_wp(WP *wp){
  if(wp == NULL) assert(0);
  WP *t = head;
  if(wp == head){
    head = NULL;
  }else{
    while(t->next != wp){
      t = t->next;
    }
    t = wp->next;
  } 
  wp->next = NULL;
  if(free_ == NULL){
    free_ = wp;
  }else{
    WP *p = free_;
    while(p->next != NULL){
      p = p->next;
    }
    p->next = wp;
  }
}

bool is_changed(){
  bool success = false;
  WP *wp = head;
  while(wp != NULL){
    word_t value = expr(wp->expr , &success);
    if(value == wp->value){
      return false;
    }else{
      return true;
    }
    wp = wp->next;
  }
  return false;
}

void delete_wp(int NO){
  WP *wp = head;
  if(head != NULL){
    while(wp != NULL){
      if(NO == wp->NO){
        free_wp(wp);
        printf("Successfully delete watchpoint %d, %s",NO , wp->expr);
        return ;
      }
    }
    wp = wp->next;
  }
}

void print_wp(){
  WP *wp = head;
  if(wp == NULL){
    printf("No watchpoint.\n");
  }else{
    while(wp != NULL){
      printf("%d ,%s\n" , wp->NO , wp->expr);
      wp = wp->next;
    }
  }
}