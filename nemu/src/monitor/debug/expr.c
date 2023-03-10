#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <memory/paddr.h>
#include <regex.h>
#include <stdlib.h>
#include "../../isa/x86/local-include/reg.h"

//uint32_t isa_reg_str2val(const char *s , bool *success);
uint32_t vaddr_read(paddr_t addr , int len);
//int hex2dec(char *input_hex);

bool check_parenthesis(int p , int q);
int eval(int p , int q);
int priority(int type);
int find_dominant_operator(int p , int q);
uint32_t get_num(char str);

//TK_MINUS 
//TK_DER memory address
enum {
  TK_NOTYPE = 256, 
  TK_EQ ,
  TK_HEX, 
  TK_NUM ,
  TK_REG, 
  TK_AND, 
  TK_OR, 
  TK_NEQ, 
  TK_MINUS, 
  TK_DER,
  TK_LE,
  TK_BE,
  TK_LESS,
  TK_BIGGER,
  TK_LM,
  TK_RM
  /* TODO: Add more token types */
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
   {"!=" ,TK_NEQ},
  {"\\-", '-'},
  {"\\*" , '*'},
  {"\\/" , '/'},

  {"0x[0-9a-f]+" , TK_HEX},
  {"[0-9]+" , TK_NUM},
  {"\\(" , '('},
  {"\\)" , ')'},
  {"\\$[a-z]{2,3}" , TK_REG},
  

  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"!" , '!'},
  {"<=",TK_LE},     //less equal
  {">=" , TK_BE},   //bigger equal
  {"<" , TK_LESS},  //less
  {">" , TK_BIGGER},//bigger
  {"<<" , TK_LM} ,  //left move
  {">>" , TK_RM}    //right move
 
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
          case '+':
          case '-':
          case '*':
          case '/':
          case '(':
          case ')': break;
          case TK_NUM:
            strncpy(tokens[nr_token].str , substr_start , substr_len);
            //printf("tokens[%d]_str=%s\n" , nr_token , tokens[nr_token].str);break;
          case TK_HEX:
            strncpy(tokens[nr_token].str , substr_start , substr_len);
            //printf("tokens[%d]_str=%s\n" , nr_token , tokens[nr_token].str);break;
          case TK_REG:
            strncpy(tokens[nr_token].str , substr_start , substr_len);
            //printf("tokens[%d]_str=%s\n" , nr_token , tokens[nr_token].str);break;
          default: break;
        }
        //printf("tokens[%d]_str%s\n" , nr_token , tokens[nr_token].str);
        nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  assert(nr_token > 0);
  printf("token size is %d\n" , nr_token);
  
  for (i = 0; i < nr_token; i++) {
		if (tokens[i].type == '-' && (i == 0 || 
    (tokens[i-1].type != ')' && tokens[i-1].type != TK_NUM && tokens[i-1].type != TK_HEX && tokens[i-1].type != TK_REG))) {
			tokens[i].type = TK_MINUS;
		}

		if (tokens[i].type == '*' && (i == 0 || 
    (tokens[i-1].type != ')' && tokens[i-1].type != TK_NUM && tokens[i-1].type != TK_HEX && tokens[i-1].type != TK_REG))) {
			tokens[i].type = TK_DER;
		}
	}
  return true;
}



int eval(int p , int q){
  if(p > q){
    printf("Bad expression.\n");
    assert(0);
  }else if(p == q){
    bool flag = false;
    int res;
    switch (tokens[p].type)
    {
      case TK_NUM: return atoi(tokens[p].str);
      case TK_HEX: sscanf(tokens[p].str , "%x" , &res); return res;
      case TK_REG: return isa_reg_str2val(tokens[p].str , &flag);
    }
  }else if(check_parenthesis(p,q) == true){
    return eval(p+1 , q-1);
  }else{
    int op;
    op = find_dominant_operator(p , q);
    if(op == -1) assert(0);

    if(tokens[op].type == TK_MINUS){
      return eval(op+1 , q)*(-1);
    }
    if(tokens[op].type == TK_DER){
      return paddr_read(eval(op+1 , q) , 4);
    }
    if(tokens[op].type == '!'){
      return !eval(op+1 , q);
    }

    word_t val1 = eval(p , op-1);
    word_t val2 = eval(op+1 , q);
    if(tokens[op].type == '/' && val2 == 0) assert(0);
    switch (tokens[op].type)
    {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1/val2;
      case TK_AND: return val1 && val2;
      case TK_OR: return val1 || val2;
      case TK_EQ: return val1 == val2;
      case TK_NOTYPE: return val1 != val2;
      case TK_LM: return (val1 << val2);
      case TK_RM: return (val1 >> val2);
      case TK_LESS: return val1 < val2;
      case TK_BIGGER: return val1 > val2;
      case TK_LE: return val1 <= val2;
      case TK_BE: return val1 >= val2;
      default: assert(0);
    }
  }
  return 0;
}

bool check_parenthesis(int p , int q){
  int i , nums = 0;
  if(tokens[p].type != '(' || tokens[q].type != ')') return false;
  for(i = p ; i <= q ; i++){
    if(tokens[i].type == '('){
      nums++;
    }
    if(tokens[i].type == ')'){
      nums--;
    }
    if(nums == 0 && i < q) return false;
  }
  if(nums != 0) return false;
  return true;
}

int find_dominant_operator(int p , int q){
  int i , nums = 0, now_position = -1;
  for(i = p ; i <= q ; i++){
    if(tokens[i].type == '('){
      nums++;
    }else if(tokens[i].type == ')'){
      nums--;
    }else if(priority(tokens[i].type) <= priority(tokens[now_position].type) && nums == 0){
      now_position = i;
    }else{
      continue;
    }
  }
  return now_position;
}

int priority(int type){
  switch (type)
  {
    case TK_MINUS: case TK_DER: return 8;
    case '*': case '/': return 7;
    case '+': case '-': return 6;
    case TK_LM: case TK_RM: return 5;
    case TK_LE: case TK_BE: case TK_LESS: case TK_BIGGER: return 4;
    case TK_NOTYPE: case TK_EQ: return 3;
    case TK_AND: return 2;
    case TK_OR: return 1;
  }
  return 20;
}

uint32_t get_num(char str){
  if(str >= '0' && str <= '9'){
    return str-'0';
  }
  else if(str >= 'a' && str <= 'f'){
    return str - 'a' + 10;
  }else if(str >= 'A' && str <= 'F'){
    return str - 'A' + 10;
  }
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  
  /* TODO: Insert codes to evaluate the expression. */
  
   
  return eval(0 , nr_token-1);
}
