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

#define LOAD_MASTER_KEY \
    LDMIA  R0!, {R9-R12} EOL\
    MOV    R0, R2        EOL\
//NEW APPROACH
#define ADD_U8(A, B) \
    UADD8 A, A, B EOL\
    /*
    //AND TT0, B, 0x7F7F7F7F EOL\
    //ADD TT0, A, TT0      EOL\
    //AND A, B, 0x80808080 EOL\
    //EOR A, A, TT0        EOL\
    */
/*
#define ADD_U8(A, B) \
    AND TT0, B, MASK02 EOL\
    ADD TT0, A, TT0     EOL\
    AND TT0, MASK02    EOL\
    AND TT1, B, MASK13 EOL\
    ADD  A, TT1        EOL\
    AND  A, MASK13    EOL\
    ORR  A, TT0        EOL\
*/
#define EKS_ROUND \
    LDMIA R0!, {R2-R5} EOL\
    ADD_U8(R2,  R9)       \
    ADD_U8(R3, R10)       \
    ADD_U8(R4, R11)       \
    ADD_U8(R5, R12)       \
    STMIA R1!, {R2-R5} EOL\

#define ROT64_R8(A, B) \
   MOV  A, A, ROL(8) EOL\
   MOV  B, B, ROL(8) EOL\
   EOR TT0, A, B      EOL\
   AND TT0, MASK0     EOL\
   EOR  A, TT0        EOL\
   EOR  B, TT0        EOL\

#define EKS_1ROUND \
    EKS_ROUND \
    ROT64_R8(R9, R10) \
    ROT64_R8(R11, R12) \


void __attribute__ ((noinline, naked)) key_asm(u8* input, u8* round_key,  u8* delta) {
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
STRFY(LOAD_MASTER_KEY)

// whitening key
    "STMIA R1!, {R12} \n\t"
// whitening key
    "STMIA R1!, {R9} \n\t"

    "MOV R6, #8 \n\t"
    "1:"
      STRFY(EKS_1ROUND)
      "SUBS R6, #1 \n\t"
      "BNE  1b     \n\t"

STRFY(ENC_EPILOG)
/////////////////////////

  :
  : 
  : "cc", "memory"
);
}
