#define STR(...) #__VA_ARGS__
#define STRFY(...) STR(__VA_ARGS__)

#define EOL \n\t

#define CS #
#define CV(X) CS(X)
#define ROL(X) ROR CV(32-X)
#define ROR(X) ROR CV(X)

#define MASK0  #0x000000FF
#define MASK02 #0x00FF00FF
#define MASK13 #0xFF00FF00

#define TT0 R7
#define TT1 R8

#define ENC_PROLOG \
 PUSH {R4-R7,R14}      EOL\
 MOV R4, R8            EOL\
 MOV R5, R9            EOL\
 MOV R6, R10           EOL\
 MOV R7, R11           EOL\
 PUSH {R4-R7}          EOL\

#define ENC_EPILOG \
 POP {R4-R7}           EOL\
 MOV R8, R4            EOL\
 MOV R9, R5            EOL\
 MOV R10, R6           EOL\
 MOV R11, R7           EOL\
 POP {R4-R7,PC}        EOL\

#define DUP \
    LDMIA  R0!, {R2}          EOL\
    AND R3, R2, #0x00FF00FF   EOL\
    AND R4, R2, #0xFF00FF00   EOL\
    ORR R3, R3, R3, LSL#8     EOL\
    ORR R4, R4, R4, LSR#8     EOL\
    STM R1!, {R3-R4}          EOL\


void __attribute__ ((noinline, naked)) key_dup(u8* input, u8* outputx2) {
asm volatile(\
/******************************************************************************
 * Macros for EKS
 *
 *        r0: master key pointer -> delta pointer
 *        r1: round key pointer
 *    r2~ r5: delta
 *        r6: loop counter
 *    r7~ r8: temp
 *    r9~r12: round keys
 *****************************************************************************/

STRFY(ENC_PROLOG)

    "MOV R6, #34 \n\t"
    "1:"
      STRFY(DUP)
    
      "SUBS R6, #1 \n\t"
      "BNE  1b     \n\t"

STRFY(ENC_EPILOG)
/////////////////////////

  :
  : 
  : "cc", "memory"
);
}
