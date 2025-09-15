#ifndef EMU_H
#define EMU_H
#endif

/*
8080 instruction encoding:

Conventions in instruction source:
    D   = Destination register (8 bit)
    S   = Source register (8 bit)
    RP  = Register pair (16 bit)
    #   = 8 or 16 bit immediate operand
    a   = 16 bit Memory address
    p   = 8 bit port address
    ccc = Conditional

Conventions in instruction encoding:
    db  = Data byte (8 bit)
    lb  = Low byte of 16 bit value
    hb  = High byte of 16 bit value
    pa  = Port address (8 bit)

Dest and Source reg fields:
    111=A   (Accumulator)
    000=B
    001=C
    010=D
    011=E
    100=H
    101=L
    110=M   (Memory reference through address in H:L)

Register pair 'RP' fields:
    00=BC   (B:C as 16 bit register)
    01=DE   (D:E as 16 bit register)
    10=HL   (H:L as 16 bit register)
    11=SP   (Stack pointer, refers to PSW (FLAGS:A) for PUSH/POP)

Condition code 'CCC' fields: (FLAGS: S Z x A x P x C)
    000=NZ  ('Z'ero flag not set)
    001=Z   ('Z'ero flag set)
    010=NC  ('C'arry flag not set)
    011=C   ('C'arry flag set)
    100=PO  ('P'arity flag not set - ODD)
    101=PE  ('P'arity flag set - EVEN)
    110=P   ('S'ign flag not set - POSITIVE)
    111=M   ('S'ign flag set - MINUS)

Inst      Encoding          Flags   Description
----------------------------------------------------------------------
MOV D,S   01DDDSSS          -       Move register to register
MVI D,#   01DDD110 db       -       Move immediate to register
LXI RP,#  00RP0001 lb hb    -       Load register pair immediate
LDA a     00111010 lb hb    -       Load A from memory
STA a     00110010 lb hb    -       Store A to memory
LHLD a    00101010 lb hb    -       Load H:L from memory
SHLD a    00100010 lb hb    -       Store H:L to memory
LDAX RP   00RP1010 *1       -       Load indirect through BC or DE
STAX RP   00RP0010 *1       -       Store indirect through BC or DE
XCHG      11101011          -       Exchange DE and HL content
ADD S     10000SSS          ZSPCA   Add register to A
ADI #     11000110 db       ZSCPA   Add immediate to A
ADC S     10001SSS          ZSCPA   Add register to A with carry
ACI #     11001110 db       ZSCPA   Add immediate to A with carry
SUB S     10010SSS          ZSCPA   Subtract register from A
SUI #     11010110 db       ZSCPA   Subtract immediate from A
SBB S     10011SSS          ZSCPA   Subtract register from A with borrow
SBI #     11011110 db       ZSCPA   Subtract immediate from A with borrow
INR D     00DDD100          ZSPA    Increment register
DCR D     00DDD101          ZSPA    Decrement register
INX RP    00RP0011          -       Increment register pair
DCX RP    00RP1011          -       Decrement register pair
DAD RP    00RP1001          C       Add register pair to HL (16 bit add)
DAA       00100111          ZSPCA   Decimal Adjust accumulator
ANA S     10100SSS          ZSCPA   AND register with A
ANI #     11100110 db       ZSPCA   AND immediate with A
ORA S     10110SSS          ZSPCA   OR  register with A
ORI #     11110110          ZSPCA   OR  immediate with A
XRA S     10101SSS          ZSPCA   ExclusiveOR register with A
XRI #     11101110 db       ZSPCA   ExclusiveOR immediate with A
CMP S     10111SSS          ZSPCA   Compare register with A
CPI #     11111110          ZSPCA   Compare immediate with A
RLC       00000111          C       Rotate A left
RRC       00001111          C       Rotate A right
RAL       00010111          C       Rotate A left through carry
RAR       00011111          C       Rotate A right through carry
CMA       00101111          -       Compliment A
CMC       00111111          C       Compliment Carry flag
STC       00110111          C       Set Carry flag
JMP a     11000011 lb hb    -       Unconditional jump
Jccc a    11CCC010 lb hb    -       Conditional jump
CALL a    11001101 lb hb    -       Unconditional subroutine call
Cccc a    11CCC100 lb hb    -       Conditional subroutine call
RET       11001001          -       Unconditional return from subroutine
Rccc      11CCC000          -       Conditional return from subroutine
RST n     11NNN111          -       Restart (Call n*8)
PCHL      11101001          -       Jump to address in H:L
PUSH RP   11RP0101 *2       -       Push register pair on the stack
POP RP    11RP0001 *2       *2      Pop  register pair from the stack
XTHL      11100011          -       Swap H:L with top word on stack
SPHL      11111001          -       Set SP to content of H:L
IN p      11011011 pa       -       Read input port into A
OUT p     11010011 pa       -       Write A to output port
EI        11111011          -       Enable interrupts
DI        11110011          -       Disable interrupts
HLT       01110110          -       Halt processor
NOP       00000000          -       No operation

*1 = Only RP=00(BC) and 01(DE) are allowed for LDAX/STAX

*2 = RP=11 refers to PSW for PUSH/POP (cannot push/pop SP).
     When PSW is POP'd, ALL flags are affected.
*/

#include <stdint.h>
typedef struct {
  uint8_t z : 1;  // zero
  uint8_t s : 1;  // sign
  uint8_t p : 1;  // parity
  uint8_t ac : 1; // aux carry for binary coded decimal arithmetic
  uint8_t cy : 1;  // carry flag for add
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

#define MEMORY_SIZE 0x4000
#define ROM_SIZE 0x2000
#define STACK_START 0x2400 // grows down in memory
#define PROGRAM_START 0x0000

#define MEM_REGISTER 6

#define GENERATE_4_CASES(a, b, c, d)                                           \
  case a:                                                                      \
  case b:                                                                      \
  case c:                                                                      \
  case d:

#define GENERATE_8_CASES(a, b, c, d, e, f, g, h)                               \
  case a:                                                                      \
  case b:                                                                      \
  case c:                                                                      \
  case d:                                                                      \
  case e:                                                                      \
  case f:                                                                      \
  case g:                                                                      \
  case h:

#define Parity(x)
    x
