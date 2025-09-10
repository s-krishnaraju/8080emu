#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "emu.h"

void printByte(uint8_t x) {
  int num_bits = 8;
  for (int i = num_bits - 1; i >= 0; i--) {
    if ((x >> i) & 1) {
      printf("1");
    } else {
      printf("0");
    }
  }
  printf("\n");
}

/* handle
  xxxxxSSS | S = src register

  xxDDDxxx | D = dest register

  01DDDSSS if last two bits 01 then it's MOV

  xxRPxxxx | RP = register pair

  xxCCCxxx | C = condition flag

  RST 0...7
  11NNN111 | NNN = 0...7
*/

int handleOpcode(uint8_t *codebuffer, int pc) {
  uint8_t *code = &codebuffer[pc];
  int opbytes = 1;

  switch (code[0]) {
  // LDA
  case 0x3a:
    printf("LDA");
    break;
  // STA
  case 0x32:
    printf("STA");
    break;
  // LHLD
  case 0x2a:
    printf("LHLD");
    break;
  // SHLD
  case 0x22:
    printf("SHLD");
    break;
  // XCHG
  case 0xeb:
    printf("XCHG");
    break;
  // ADI
  case 0xc6:
    printf("ADI");
    break;
  // ACI
  case 0xce:
    printf("ACI");
    break;
  // SUI
  case 0xd6:
    printf("SUI");
    break;
  // SBI
  case 0xde:
    printf("SBI");
    break;
  // DAA
  case 0x27:
    printf("DAA");
    break;
  // ANI
  case 0xe6:
    printf("ANI");
    break;
  // ORI
  case 0xf6:
    printf("ORI");
    break;
  // XRI
  case 0xee:
    printf("XRI");
    break;
  // RLC
  case 0x07:
    printf("RLC");
    break;
  // RRC
  case 0x0f:
    printf("RRC");
    break;
  // RAL
  case 0x17:
    printf("RAL");
    break;
  // RAR
  case 0x1f:
    printf("RAR");
    break;
  // CMA
  case 0x2f:
    printf("CMA");
    break;
  // CMC
  case 0x3f:
    printf("CMC");
    break;
  // JMP
  case 0xc3:
    printf("JMP");
    break;
  // CALL
  case 0xcd:
    printf("CALL");
    break;
  // RET
  case 0xc9:
    printf("RET");
    break;
  // PCHL
  case 0xe9:
    printf("PCHL");
    break;
  // XTHL
  case 0xe3:
    printf("XTHL");
    break;
  // SPHL
  case 0xf9:
    printf("SPHL");
    break;
  // EI
  case 0xfb:
    printf("EI");
    break;
  // DI
  case 0xf3:
    printf("DI");
    break;
  // HLT
  case 0x76:
    printf("HLT");
    break;
  // IN
  case 0xdb:
    printf("IN");
    break;
  // OUT
  case 0xd3:
    printf("OUT");
    break;
  default:
    break;
  }
  uint8_t last_two_bits = code[0] >> 6;
  uint8_t first_three_bits = code[0] & 7;

  switch (last_two_bits) {
  case 1:
    printf("MOV");
    opbytes = 3;
    printByte(last_two_bits);
    break;
  }

  printf("\n");
  return opbytes;
}

int main(int argc, char *argv[]) {
  FILE *f = fopen(argv[1], "rb");

  if (f == NULL) {
    printf("error: Couldn't open file %s\n", argv[1]);
    exit(1);
  }

  fseek(f, 0, SEEK_END);
  int fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  unsigned char *buffer = malloc(fsize);
  fread(buffer, fsize, 1, f);
  fclose(f);

  int pc = 0;

  // TODO: init everything here
  // make the CPUState, allocate ram/rom, etc ...

  while (pc < fsize) {
    pc += handleOpcode(buffer, pc);
  }
  return 0;
}
