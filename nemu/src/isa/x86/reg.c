#include <isa.h>
#include <stdlib.h>
#include <time.h>
#include "local-include/reg.h"

const char *regsl[] = {"$eax", "$ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

void reg_test() {
  srand(time(0));
  word_t sample[8];
  word_t pc_sample = rand();
  cpu.pc = pc_sample;

  int i;
  for (i = R_EAX; i <= R_EDI; i ++) {
    sample[i] = rand();
    reg_l(i) = sample[i];
    assert(reg_w(i) == (sample[i] & 0xffff));
  }

  assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
  assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
  assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
  assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
  assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
  assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
  assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
  assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));

  assert(sample[R_EAX] == cpu.eax);
  assert(sample[R_ECX] == cpu.ecx);
  assert(sample[R_EDX] == cpu.edx);
  assert(sample[R_EBX] == cpu.ebx);
  assert(sample[R_ESP] == cpu.esp);
  assert(sample[R_EBP] == cpu.ebp);
  assert(sample[R_ESI] == cpu.esi);
  assert(sample[R_EDI] == cpu.edi);

  assert(pc_sample == cpu.pc);
}

void isa_reg_display() {
  // for(int i = 0 ; i < 8 ; i++){
  //   printf("%s\t %x\n" , regsl[i] , cpu.gpr[i]._32);
  // }

  printf("%%eax: 0x%-12x",cpu.eax);
  printf("%%ax: 0x%-12x",cpu.eax & 0xffff);
  printf("%%ah: 0x%-12x",(cpu.eax >> 8) & 0xff);
  printf("%%al: 0x%-12x\n",cpu.eax & 0xff);
  
  printf("%%ebx: 0x%-12x",cpu.ebx);
  printf("%%bx: 0x%-12x",cpu.ebx & 0xffff);
  printf("%%bh: 0x%-12x",(cpu.ebx >> 8) & 0xff);
  printf("%%bl: 0x%-12x\n",cpu.ebx & 0xff);
  
  printf("%%ecx: 0x%-12x",cpu.ecx);
  printf("%%cx: 0x%-12x",cpu.ecx & 0xffff);
  printf("%%ch: 0x%-12x",(cpu.ecx >> 8) & 0xff);
  printf("%%cl: 0x%-12x\n",cpu.ecx & 0xff);
  
  printf("%%edx: 0x%-12x",cpu.edx);
  printf("%%dx: 0x%-12x",cpu.edx & 0xffff);
  printf("%%dh: 0x%-12x",(cpu.edx >> 8) & 0xff);
  printf("%%dl: 0x%-12x\n",cpu.edx & 0xff);
  
  printf("%%ebp: 0x%-12x",cpu.ebp);
  printf("%%bp: 0x%-12x",cpu.ebp & 0xffff);
  // printf("%%bph: 0x%-11x",(cpu.ebp >> 8) & 0xff);
  // printf("%%bpl: 0x%-11x\n",cpu.ebp & 0xff);
  
  printf("%%esi: 0x%-12x",cpu.esi);
  printf("%%si: 0x%-12x",cpu.esi & 0xffff);
  // printf("%%sih: 0x%-11x",(cpu.esi >> 8) & 0xff);
  // printf("%%sil: 0x%-11x\n",cpu.esi & 0xff);
  
  printf("%%edi: 0x%-12x",cpu.edi);
  printf("%%di: 0x%-12x",cpu.edi & 0xffff);
  // printf("%%dih: 0x%-11x",(cpu.edi >> 8) & 0xff);
  // printf("%%dil: 0x%-11x\n",cpu.edi & 0xff);
  
  printf("%%esp: 0x%-12x",cpu.esp);
  printf("%%sp: 0x%-12x",cpu.esp & 0xffff);
  // printf("%%sph: 0x%-11x",(cpu.esp >> 8) & 0xff);
  // printf("%%spl: 0x%-11x\n",cpu.esp & 0xff);
}

word_t isa_reg_str2val(const char *s, bool *success) {
  char *pc ="pc";
  if (strcmp(s,pc) == 0)
    return cpu.pc;
  int which_set = -1;
  int reg_index = 0;
  uint32_t reg_array[] = {cpu.eax, cpu.ecx, cpu.edx, cpu.ebx, cpu.esp, cpu.ebp, cpu.esi, cpu.edi};
  for (int i = 0; i < 8; i++){
    if (strcmp(s,regsl[i]) == 0){
      which_set = 1;
      reg_index = i;
      break;
    }
    if (strcmp(s,regsw[i]) == 0){
      which_set = 2;
      reg_index = i;
      break;
    }
    if (strcmp(s,regsb[i]) == 0){
      which_set = 3;
      reg_index = i;
      break;
    }
  }
  assert(!(which_set == -1));
  switch (which_set){
    case 1: return reg_array[reg_index];
    case 2: return reg_array[reg_index] & 0xffff;
    case 3: return reg_array[reg_index] & 0xff;
  }
  return -1;
}
