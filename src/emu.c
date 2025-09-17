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
}

void unimplementedOpcodeError(uint8_t opcode) {
  printf("ERROR UNIMPLEMENTED OPCODE: ");
  printByte(opcode);
  printf("\n");
  exit(EXIT_FAILURE);
}

static inline uint16_t get16Bit(uint8_t hb, uint8_t lb) {
  return (hb << 8) | lb;
}

static inline uint8_t getMReg(CPUState *state) {
  uint16_t addr = get16Bit(state->h, state->l);
  return state->memory[addr];
}

static inline void setMReg(CPUState *state, uint8_t data) {
  uint16_t addr = get16Bit(state->h, state->l);
  state->memory[addr] = data;
}

static inline uint8_t parity(uint8_t x) {
  uint8_t n_one = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (x >> i)
      n_one += 1;
  }
  return (n_one & 1) == 0;
}

static inline uint8_t sign(uint8_t x) { return (x & 0x80) != 0; }

static inline void i8080_lda(CPUState *state, uint8_t hb, uint8_t lb) {
  uint16_t addr = get16Bit(hb, lb);
  state->a = state->memory[addr];
  state->pc += 3;
}

static inline void i8080_sta(CPUState *state, uint8_t hb, uint8_t lb) {
  uint16_t addr = get16Bit(hb, lb);
  state->memory[addr] = state->a;
  state->pc += 3;
}

static inline void i8080_lhld(CPUState *state, uint8_t hb, uint8_t lb) {
  uint16_t addr = get16Bit(hb, lb);
  state->l = state->memory[addr];
  state->h = state->memory[addr + 1];
  state->pc += 3;
}

static inline void i8080_shld(CPUState *state, uint8_t hb, uint8_t lb) {
  uint16_t addr = get16Bit(hb, lb);
  state->memory[addr] = state->l;
  state->memory[addr + 1] = state->h;
  state->pc += 3;
}

static inline void i8080_xchg(CPUState *state) {
  uint8_t tmp = state->h;
  state->h = state->d;
  state->d = tmp;
  tmp = state->l;
  state->l = state->e;
  state->e = tmp;
  state->pc += 1;
}

static inline void i8080_adi(CPUState *state, uint8_t db, uint8_t carry) {
  uint16_t val = state->a + db + carry;

  state->cc.z = (val & 0xff) == 0;
  state->cc.s = sign(val);
  state->cc.cy = val > 0xff;
  state->cc.p = parity(val);
  state->cc.ac = ((state->a & 0x08) || (db & 0x08)) && (!(val & 0x08));

  state->a = val & 0xff;
  state->pc += 2;
}

static inline void i8080_sui(CPUState *state, uint8_t db, uint8_t carry) {
  uint8_t val = state->a + ((~(db + carry)) + 1);

  state->cc.cy = (db + carry) > state->a;
  state->cc.s = sign(val);
  state->cc.z = val == 0;
  state->cc.p = parity(val);

  state->a = val;
  state->pc += 2;
}

static inline void i8080_daa(CPUState *state) {
  uint8_t nib1 = state->a & 0x0f;
  uint8_t nib2 = state->a >> 4;

  if (nib1 > 9 || state->cc.ac)
    nib1 += 6;

  if (nib2 > 9 || state->cc.cy)
    nib2 += 6;

  uint8_t val = nib1 + (nib2 << 4);
  state->cc.ac = (nib1 & 0xf0) != 0;
  state->cc.cy = (nib2 & 0xf0) != 0;
  state->cc.z = val == 0;
  state->cc.p = parity(val);
  state->cc.s = sign(val);

  state->a = val;
  state->pc += 1;
}

static inline void i8080_mov(CPUState *state, uint8_t opcode,
                             uint8_t *registers[]) {
  uint8_t dest_reg = (opcode >> 3) & 7;
  uint8_t src_reg = opcode & 7;

  if (src_reg == MEM_REGISTER) {
    *registers[dest_reg] = getMReg(state);
  } else if (dest_reg == MEM_REGISTER) {
    setMReg(state, *registers[src_reg]);
  } else {
    *registers[dest_reg] = *registers[src_reg];
  }
  state->pc += 1;
}

static inline void i8080_mvi(CPUState *state, uint8_t opcode, uint8_t db,
                             uint8_t *registers[]) {
  uint8_t dest_reg = (opcode >> 3) & 7;

  if (dest_reg == MEM_REGISTER) {
    setMReg(state, db);
  } else {
    *registers[dest_reg] = db;
  }
  state->pc += 2;
}

static inline void i8080_lxi(CPUState *state, uint8_t *reg1, uint8_t *reg2,
                             uint8_t hb, uint8_t lb) {
  *reg1 = hb;
  *reg2 = lb;
  state->pc += 3;
}

static inline void i8080_lxi_sp(CPUState *state, uint8_t hb, uint8_t lb) {
  state->sp = get16Bit(hb, lb);
  state->pc += 3;
}

static inline void i8080_add(CPUState *state, uint8_t opcode,
                             uint8_t *registers[], uint8_t carry) {
  uint8_t reg = opcode & 7;
  uint8_t db;
  if (reg == MEM_REGISTER) {
    db = getMReg(state);
  } else {
    db = *registers[reg];
  }
  uint16_t val = state->a + db + carry;

  state->cc.z = (val & 0xff) == 0;
  state->cc.s = sign(val);
  state->cc.cy = val > 0xff;
  state->cc.p = parity(val);
  state->cc.ac = ((state->a & 0x08) || (db & 0x08)) && ((val & 0x08) == 0);

  state->a = val & 0xff;
  state->pc += 1;
}

static inline void i8080_sub(CPUState *state, uint8_t opcode,
                             uint8_t *registers[], uint8_t carry) {

  uint8_t reg = opcode & 7;
  uint8_t db;
  if (reg == MEM_REGISTER) {
    db = getMReg(state);
  } else {
    db = *registers[reg];
  }
  db += carry;
  uint8_t val = state->a + ((~db) + 1);
  state->cc.cy = db > state->a;
  state->cc.s = sign(val);
  state->cc.z = val == 0;
  state->cc.p = parity(val);

  state->a = val;
  state->pc += 1;
}

void handleOpcode(CPUState *state, uint8_t *registers[]) {
  uint8_t *code = &state->memory[state->pc];

  switch (code[0]) {
  // LDA
  case 0x3a:
    i8080_lda(state, code[2], code[1]);
    break;
  // STA
  case 0x32:
    i8080_sta(state, code[2], code[1]);
    break;
  // LHLD
  case 0x2a:
    i8080_lhld(state, code[2], code[1]);
    break;
  // SHLD
  case 0x22:
    i8080_shld(state, code[2], code[1]);
    break;
  // XCHG
  case 0xeb:
    i8080_xchg(state);
    break;
  // ADI
  case 0xc6:
    i8080_adi(state, code[1], 0);
    break;
  // ACI
  case 0xce:
    i8080_adi(state, code[1], state->cc.cy);
    break;
  // SUI
  case 0xd6:
    i8080_sui(state, code[1], 0);
    break;
  // SBI
  case 0xde:
    i8080_sui(state, code[1], state->cc.cy);
    break;
  // DAA
  case 0x27:
    i8080_daa(state);
    break;
  // STC
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
    break;

  // MOV
  case 0x40 ... 0x75:
  case 0x77 ... 0x7f:
    i8080_mov(state, code[0], registers);
    break;

  // MVI
  case 0x06:
  case 0x0e:
  case 0x16:
  case 0x1e:
  case 0x26:
  case 0x2e:
  case 0x36:
  case 0x3e:
    i8080_mvi(state, code[0], code[1], registers);
    break;

  // LXI
  case 0x01: // bc
    i8080_lxi(state, &state->b, &state->c, code[2], code[1]);
    break;
  case 0x11: // de
    i8080_lxi(state, &state->d, &state->e, code[2], code[1]);
    break;
  case 0x21: // hl
    i8080_lxi(state, &state->h, &state->l, code[2], code[1]);
    break;
  case 0x31: // sp
    i8080_lxi_sp(state, code[2], code[1]);
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
  case 0x80 ... 0x87: {
    i8080_add(state, code[0], registers, 0);
    break;
  }

  // ADC
  case 0x88 ... 0x8f: {
    i8080_add(state, code[0], registers, state->cc.cy);
    break;
  }

  // SUB
  case 0x90 ... 0x97: 
    i8080_sub(state, code[0], registers, 0);
    break;
  

  // SBB
  case 0x98 ... 0x9f: 
    i8080_sub(state, code[0], registers, state->cc.cy);
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
  case 0x04:
  case 0x0c:
  case 0x14:
  case 0x1c:
  case 0x24:
  case 0x2c:
  case 0x34:
  case 0x3c: {
    printf("INR");
    uint8_t reg = (code[0] >> 3) & 7;
    uint8_t val;
    uint8_t tmp;
    if (reg == MEM_REGISTER) {
      tmp = getMReg(state);
      val = tmp + 1;
      setMReg(state, val);
    } else {
      tmp = *registers[reg];
      val = tmp + 1;
      *registers[reg] = val;
    }
    state->cc.z = (val == 0);
    state->cc.s = (sign(val));
    state->cc.p = parity(val);
    state->cc.ac = (tmp & 0x08) && (!(val & 0x08));
    break;
  }

  // DCR
  case 0x05:
  case 0x0d:
  case 0x15:
  case 0x1d:
  case 0x25:
  case 0x2d:
  case 0x35:
  case 0x3d: {
    printf("DCR");
    uint8_t reg = (code[0] >> 3) & 7;
    uint8_t val;
    if (reg == MEM_REGISTER) {
      val = getMReg(state) - 1;
      setMReg(state, val);
    } else {
      val = *registers[reg] - 1;
      *registers[reg] = val;
    }
    state->cc.z = val == 0;
    state->cc.s = sign(val);
    state->cc.p = parity(val);
    break;
  }

  // INX
  case 0x03: {
    printf("INX BC");
    state->c += 1;
    // we overflowed
    if (state->c == 0) {
      state->b += 1;
    }
    break;
  }
  case 0x13:
    printf("INX DE");
    state->e += 1;
    if (state->e == 0) {
      state->d += 1;
    }
    break;
  case 0x23:
    printf("INX HL");
    state->l += 1;
    if (state->l == 0) {
      state->h += 1;
    }
    break;
  case 0x33:
    printf("INX SP");
    state->sp += 1;
    break;

  // DCX
  case 0x0b:
    printf("DCX BC");
    state->c -= 1;
    // we underflowed
    if (state->c == 0xff) {
      state->b -= 1;
    }
    break;
  case 0x1b:
    printf("DCX DE");
    state->e -= 1;
    // we underflowed
    if (state->e == 0xff) {
      state->d -= 1;
    }
    break;
  case 0x2b:
    printf("DCX HL");
    state->l -= 1;
    // we underflowed
    if (state->l == 0xff) {
      state->h -= 1;
    }
    break;
  case 0x3b:
    printf("DCX SP");
    state->sp -= 1;
    break;

  // DAD
  case 0x09: {
    printf("DAD BC");
    uint16_t val = state->l + state->c;
    state->l = val & 0xff;
    uint8_t cy = val > 0xff;
    val = state->h + state->b + cy;
    state->cc.cy = val > 0xff;
    state->h = val & 0xff;
    break;
  }
  case 0x19: {
    printf("DAD DE");
    uint16_t val = state->l + state->e;
    state->l = val & 0xff;
    uint8_t cy = val > 0xff;
    val = state->h + state->d + cy;
    state->cc.cy = val > 0xff;
    state->h = val & 0xff;
    break;
  }
  case 0x29: {
    printf("DAD HL");
    uint16_t val = state->l + state->l;
    state->l = val & 0xff;
    uint8_t cy = val > 0xff;
    val = state->h + state->h + cy;
    state->cc.cy = val > 0xff;
    state->h = val & 0xff;
    break;
  }
  case 0x39: {
    printf("DAD SP");
    uint16_t val = state->l + (state->sp & 0xff);
    state->l = val & 0xff;
    uint8_t cy = val > 0xff;
    val = state->h + (state->sp >> 8) + cy;
    state->cc.cy = val > 0xff;
    state->h = val & 0xff;
    break;
  }

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
  case 0xc7:
  case 0xcf:
  case 0xd7:
  case 0xdf:
  case 0xe7:
  case 0xef:
  case 0xf7:
  case 0xff:
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
    unimplementedOpcodeError(code[0]);
    break;
  }
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
  registers[6] = NULL; // mem reg
  registers[7] = &cpu_state.a;

  fseek(f, 0, SEEK_END);
  int fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  fread(cpu_state.memory, fsize, 1, f);
  fclose(f);

  while (cpu_state.pc < fsize) {
    handleOpcode(&cpu_state, registers);
  }
  return 0;
}
