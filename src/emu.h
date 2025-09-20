#ifndef EMU_H
#define EMU_H

#include <stdint.h>

typedef struct {
  uint8_t z : 1;  // zero
  uint8_t s : 1;  // sign
  uint8_t p : 1;  // parity
  uint8_t ac : 1; // aux carry for binary coded decimal arithmetic
  uint8_t cy : 1; // carry flag for add
  uint8_t pad : 3;
} ConditionCodes;

typedef struct {
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
  uint8_t e;
  uint8_t h;
  uint8_t l;
  uint16_t sp;
  uint16_t pc;
  uint8_t *memory;
  ConditionCodes cc;
  uint8_t int_enable;
} CPUState;


void handleOpcode(CPUState *state, uint8_t *registers[]);


#define MEMORY_SIZE 0x4000
#define ROM_SIZE 0x2000
// grows down in memory and start one after end of stack (23ff)
#define STACK_START 0x2400
#define PROGRAM_START 0x0000

#define MEM_REGISTER 6

#endif
