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

uint16_t get16Bit(uint8_t hb, uint8_t lb) { return (hb << 8) | lb; }

uint8_t getMReg(CPUState *state) {
  uint16_t addr = get16Bit(state->h, state->l);
  return state->memory[addr];
}

void setMReg(CPUState *state, uint8_t data) {
  uint16_t addr = get16Bit(state->h, state->l);
  state->memory[addr] = data;
}

int handleOpcode(CPUState *state, uint8_t *registers[]) {
  uint8_t *code = &state->memory[state->pc];
  int new_pc = state->pc + 1;

  switch (code[0]) {
  // LDA
  case 0x3a: {
    printf("LDA");
    uint16_t addr = get16Bit(code[2], code[1]);
    state->a = state->memory[addr];
    new_pc = state->pc + 3;
    break;
  }
  // STA
  case 0x32: {
    printf("STA");
    uint16_t addr = get16Bit(code[2], code[1]);
    state->memory[addr] = state->a;
    new_pc = state->pc + 3;
    break;
  }
  // LHLD
  case 0x2a: {
    printf("LHLD");
    uint16_t addr = get16Bit(code[2], code[1]);
    state->l = state->memory[addr];
    state->h = state->memory[addr + 1];
    new_pc = state->pc + 3;
    break;
  }
  // SHLD
  case 0x22: {
    printf("SHLD");
    uint16_t addr = get16Bit(code[2], code[1]);
    state->memory[addr] = state->l;
    state->memory[addr+1] = state->h;
    new_pc = state->pc + 3;
    break;
  }
  // XCHG
  case 0xeb:
    printf("XCHG");
    uint8_t tmp = state->h;
    state->h = state->d;
    state->d = tmp;
    tmp = state->l;
    state->l = state->e;
    state->e = tmp;
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
  case 0x37:
    printf("STC");
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
  // CPI
  case 0xfe:
    printf("CPI");
    new_pc = 2;
    break;

  // MOV
  case 0x40 ... 0x75:
  case 0x77 ... 0x7f: {
    printf("MOV");

    uint8_t dest_reg = (code[0] >> 3) & 7;
    uint8_t src_reg = code[0] & 7;

    if (src_reg == MEM_REGISTER) {
      dest_reg = getMReg(state);
    } else if (dest_reg == MEM_REGISTER) {
      setMReg(state, *registers[src_reg]);
    } else {
      *registers[dest_reg] = *registers[src_reg];
    }

    new_pc = state->pc + 3;
    break;
  }

  // MVI
  GENERATE_8_CASES(0x06, 0x0e, 0x16, 0x1e, 0x26, 0x2e, 0x36, 0x3e) {
    printf("MVI");
    uint8_t dest_reg = (code[0] >> 3) & 7;
    if (dest_reg == MEM_REGISTER) {
      setMReg(state, code[1]);
    } else {
      *registers[dest_reg] = code[1];
    }

    new_pc = state->pc + 2;
    break;
  }

  // LXI rp
  case 0x01: // bc
    state->b = code[2];
    state->c = code[1];
    new_pc = state->pc + 3;
    break;
  case 0x11: // de
    state->d = code[2];
    state->e = code[1];
    new_pc = state->pc + 3;
    break;
  case 0x21: // hl
    state->h = code[2];
    state->l = code[1];
    new_pc = state->pc + 3;
    break;
  case 0x31: // sp
    state->sp = get16Bit(code[2], code[1]);
    new_pc = state->pc + 3;
    break;

  // STAX
  case 0x02:
  case 0x12:
    printf("STAX");
    break;

  // LDAX
  case 0x0a:
  case 0x1a:
    printf("LDAX");
    break;

  // ADD
  case 0x80 ... 0x87:
    printf("ADD");
    uint8_t reg = code[0] & 7;
    
    break;

  // ADC
  case 0x88 ... 0x8f:
    printf("ADC");
    break;

  // SUB
  case 0x90 ... 0x97:
    printf("SUB");
    break;

  // SBB
  case 0x98 ... 0x9f:
    printf("SBB");
    break;

  // ANA
  case 0xa0 ... 0xa7:
    printf("ANA");
    break;

  // XRA
  case 0xa8 ... 0xaf:
    printf("XRA");
    break;

  // ORA
  case 0xb0 ... 0xb7:
    printf("ORA");
    break;

  // CMP
  case 0xb8 ... 0xbf:
    printf("CMP");
    break;

    // INR
    GENERATE_8_CASES(0x04, 0x0c, 0x14, 0x1c, 0x24, 0x2c, 0x34, 0x3c)
    printf("INR");
    break;

    // DCR
    GENERATE_8_CASES(0x05, 0x0d, 0x15, 0x1d, 0x25, 0x2d, 0x35, 0x3d)
    printf("DCR");
    break;

    // INX
    GENERATE_4_CASES(0x03, 0x13, 0x23, 0x33)
    printf("INX");
    break;

    // DCX
    GENERATE_4_CASES(0x0b, 0x1b, 0x2b, 0x3b)
    printf("DCX");
    break;

    // DAD
    GENERATE_4_CASES(0x09, 0x19, 0x29, 0x39)
    printf("DAD");
    break;

  // Jccc
  case 0xc2: // JNZ
  case 0xca: // JZ
  case 0xd2: // JNC
  case 0xda: // JC
  case 0xe2: // JPO
  case 0xea: // JPE
  case 0xf2: // JP
  case 0xfa: // JM
    printf("Jccc");
    new_pc = 3;
    break;

  // Cccc
  case 0xc4: // CNZ
  case 0xcc: // CZ
  case 0xd4: // CNC
  case 0xdc: // CC
  case 0xe4: // CPO
  case 0xec: // CPE
  case 0xf4: // CP
  case 0xfc: // CM
    printf("Cccc");
    new_pc = 3;
    break;

  // Rccc
  case 0xc0: // RNZ
  case 0xc8: // RZ
  case 0xd0: // RNC
  case 0xd8: // RC
  case 0xe0: // RPO
  case 0xe8: // RPE
  case 0xf0: // RP
  case 0xf8: // RM
    printf("Rccc");
    new_pc = 3;
    break;

    // RST x
    GENERATE_8_CASES(0xc7, 0xcf, 0xd7, 0xdf, 0xe7, 0xef, 0xf7, 0xff)
    printf("RST x");
    break;

  // PUSH
  case 0xc5:
  case 0xd5:
  case 0xe5:
  case 0xf5:
    printf("PUSH x");
    break;
  // POP
  case 0xc1:
  case 0xd1:
  case 0xe1:
  case 0xf1:
    printf("POP x");
    break;

  case 0x00:
  case 0x08:
  case 0x10:
  case 0x18:
  case 0x20:
  case 0x28:
  case 0x30:
  case 0x38:
  case 0xcb:
  case 0xd9:
  case 0xdd:
  case 0xed:
  case 0xfd:
    printf("NOP");
    break;

  default:
    printf("HEX: %x", code[0]);
    break;
  }

  uint8_t first_three_bits = code[0] & 7;

  printf("\n");

  return new_pc;
}

int main(int argc, char *argv[]) {
  FILE *f = fopen(argv[1], "rb");

  if (f == NULL) {
    printf("error: Couldn't open file %s\n", argv[1]);
    exit(1);
  }

  CPUState cpu_state;
  cpu_state.memory = (uint8_t *)malloc(MEMORY_SIZE);
  cpu_state.pc = PROGRAM_START;

  uint8_t *registers[8];
  registers[0] = &cpu_state.b;
  registers[1] = &cpu_state.c;
  registers[2] = &cpu_state.d;
  registers[3] = &cpu_state.e;
  registers[4] = &cpu_state.h;
  registers[5] = &cpu_state.l;
  registers[6] = NULL; // m reg
  registers[7] = &cpu_state.a;

  fseek(f, 0, SEEK_END);
  int fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  fread(cpu_state.memory, fsize, 1, f);
  fclose(f);

  while (cpu_state.pc < fsize) {
    cpu_state.pc = handleOpcode(&cpu_state, registers);
  }
  return 0;
}
