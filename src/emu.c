#include <stdio.h>
#include <stdlib.h>

#include "emu.h"

void printTwoArgs(char *s, unsigned char *code) {
  printf("%s, %02x%02x", s, code[2], code[1]);
}

void printOneArg(char *s, unsigned char *code) {
  printf("%s, %02x", s, code[1]);
}

int handleOpcode(unsigned char *codebuffer, int pc) {
  unsigned char *code = &codebuffer[pc];
  int opbytes = 1;
  // we don't need to handle each opcode, handle it bitwise and look at it to
  // get dest/source reg and stuff
  switch (code[0]) {
  // NOP
  case 0x00:
    break;
  // LXI B, code
  case 0x01:
    printTwoArgs("LXI B", code);
    opbytes = 3;
    break;
  case 0x02:
    printf("STAX B");
    break;
  case 0x03:
    printf("INX B");
    break;
  case 0x04:
    printf("INR B");
    break;
  case 0x05:
    printf("DCR B");
    break;
  case 0x06:
    printOneArg("MVI B", code);
    opbytes = 2;
    break;
  case 0x07:
    printf("RLC");
    break;
  case 0x08:
    printf("NOP");
    break;
  case 0x09:
    printf("DAD B");
    break;
  case 0x0a:
    printf("LDAX B");
    break;
  case 0x0b:
    printf("DCX B");
    break;
  case 0x0c:
    printf("INR C");
    break;
  case 0x0d:
    printf("DCR");
    break;
  case 0x0e:
    printOneArg("MVI C", code);
    opbytes = 2;
    break;
  case 0x0f:
    printf("RRC");
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

  while (pc < fsize) {
    pc += handleOpcode(buffer, pc);
  }
  return 0;
}
