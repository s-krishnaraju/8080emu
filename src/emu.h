typedef struct ConditionCodes { 


} ConditionCodes;

typedef struct CPUState {
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
    struct ConditionCodes;
    uint8_t int_enable;
} CPUState;
