#include <isa.h>
#include "expr.h"
#include "watchpoint.h"
#include "memory/paddr.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>



void cpu_exec(uint64_t);
int is_batch_mode();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
  char *arg = strtok(NULL , " ");
  if(arg == NULL){
    cpu_exec(1);
    return 0;
  }
  int N = atoi(arg);
  if(N < -1){
    return 0;
  }else{
    cpu_exec(N);
    printf("Executed %d instructions.\n" , N);
  }
  return 0;
}

static int cmd_info(char *args){
  char *arg = strtok(NULL , " ");
  switch (*arg)
  {
    case 'r':
      isa_reg_display();
      break;
      
    default:
      return 0;
  }
  return 0;
}

static int cmd_x(char *args){
  char *arg_1 = strtok(NULL , " ");
  int N = atoi(arg_1);
  char *arg_2 = strtok(NULL , " ");
  paddr_t addr_begin;
  sscanf(arg_2 , "%x" , &addr_begin);
  printf("%x\n" , addr_begin);
  for(int i = 0 ; i < N ; i++){
    printf("0x80%x\n"  , paddr_read(addr_begin , 4));
    addr_begin += 4;
  }
  return 0;
}

static int cmd_p(char *args){
  char *arg_1 = strtok(NULL , " ");
  bool *success = false;
  int ans = expr(arg_1 , success);
  if(ans == 0){
    printf("Expression fail!\n");
  }else{
    printf("Expression value is %d\n" , ans);
  }
  
  return ans;
}

// 0x1234b8
// 0x27b900
// 0x1890010
// 0x441c766
// 0x2bb0001
// 0x66000000

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si" , "Execute N steps" , cmd_si},
  { "info" , "Print system state" , cmd_info},
  { "x" , "Print N bytes memory value" , cmd_x},
  { "p" , "Expression infer value." , cmd_p}
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop() {
  if (is_batch_mode()) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
