#define STR(...) #__VA_ARGS__
#define STRFY(...) STR(__VA_ARGS__)

#define EOL \n\t

#define CS #
#define CV(X) CS(X)

#define MASK02 #0x00FF00FF

#define X0 R2
#define X1 R3
#define X2 R4
#define X3 R5

#define M02 R6

#define K0 R8
#define K1 R9

#define T0 R10
#define T1 R11
#define T2 R12

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

 #define F0(X, Y, T) \
    LSL T, X, CV(1)        EOL\
    EOR T, T, X, LSR CV(7) EOL\
    EOR T, T, X, LSL CV(2) EOL\
    EOR T, T, X, LSR CV(6) EOL\
    EOR T, T, X, LSL CV(7) EOL\
    EOR T, T, X, LSR CV(1) EOL\
    AND Y, T, MASK02       EOL\

#define F1(X, Y, T) \
    LSL T, X, CV(3)        EOL\
    EOR T, T, X, LSR CV(5) EOL\
    EOR T, T, X, LSL CV(4) EOL\
    EOR T, T, X, LSR CV(4) EOL\
    EOR T, T, X, LSL CV(6) EOL\
    EOR T, T, X, LSR CV(2) EOL\
    AND Y, T, MASK02       EOL\

#define KEYLOAD(XP, Y0, Y1, M) \
    LDM XP!, {Y0}             EOL\
    AND Y1, Y0, M, LSL CV(8)  EOL\
    AND Y0, Y0, M             EOL\
    LSR Y1, Y1, CV(8)         EOL\

#define ADD_XOR(X0, K0, X1, T) \
    UADD8 T, X0, K0        EOL\
    EOR X1, X1, T          EOL\
/*
    ADD T, X0, K0          EOL\
    EOR X1, X1, T          EOL\
    AND X1, X1, MASK02     EOL\
*/
#define XOR_ADD(X0, K0, X1, T) \
    EOR T, X0, K0          EOL\
    UADD8 X1, X1, T        EOL\
/*
    EOR T, X0, K0          EOL\
    ADD X1, X1, T          EOL\
    AND X1, X1, MASK02     EOL\
*/
#define TEXT_SHIFT(X0, X1, X2, X3, T) \
    MOV T, X3, ROR CV(16)                EOL\
    MOV X3, X2               EOL\
    MOV X2, X1               EOL\
    MOV X1, X0   EOL\
    MOV X0, T                EOL\

void __attribute__ ((noinline, naked)) enc_asm(u8* input, u8* round_num) {
asm volatile(\
/******************************************************************************
 * Macros for encryption
 *
 *      r0: block pointer
 *      r1: round key pointer
 * r2 ~ r5: block 
 *      r6: MASK01
 *      r7: loop counter
 *      r8: round key0
 *      r9: MASK0 --> round key1
 *      r10: tmp0
 *      r11: tmp1
 *      r12 : tmp2
 *****************************************************************************/
STRFY(ENC_PROLOG)



/////////////////////////
" LDM R0!, {R11-R12}	  \n\t"
" SUB R0, R0, #4*2      \n\t"

//3 2 1 0
//5 4 7 6 

" MOVW R9, #0X00FF        \n\t"
" MOVT R9, #0X0000        \n\t"
//" LDR R9,  =#0X000000FF    \n\t"

" AND R2, R11, R9         \n\t"
" AND R3, R11, R9, LSL#8  \n\t"
" AND R4, R11, R9, LSL#16 \n\t"
" AND R5, R11, R9, LSL#24 \n\t"

//- 4 - 0
" AND R11, R12, R9          \n\t"
" ORR R2, R2, R11, LSL#16   \n\t"

//5 - 1 -
" AND R11, R12, R9, LSL#8   \n\t"
" ORR R3, R3, R11, LSL#16   \n\t"
//- 5 - 1
" LSR R3, R3, #8            \n\t"

//- 2 - 6
" AND R11, R12, R9, LSL#16  \n\t"
" ORR R4, R4, R11, LSR#16   \n\t"
//- 6 - 2
" ROR R4, R4, #16           \n\t"

//3 - 7 -
" AND R11, R12, R9, LSL#24  \n\t"
" ORR R5, R5, R11, LSR#16   \n\t"
//- 7 - 3
" ROR R5, R5, #24           \n\t"


// 7 6 5 4 3 2 1 0
//
//" LDR R6,  =#0X00FF00FF  \n\t"//SET M02
" MOVW R6, #0X00FF        \n\t"
" MOVT R6, #0X00FF        \n\t"

STRFY( KEYLOAD(R1, K0, K1, M02) )
" ADD R2, R2, R8        \n\t"
" EOR R4, R4, R9        \n\t"
" AND R2, R2, R6        \n\t"

" ADD R1, R1, #4*1      \n\t"




" MOV R7, #32            \n\t"
"1:"
//ROUND
STRFY( KEYLOAD(R1, K0, K1, M02) )

STRFY( F1(X0, T0, T1) )
STRFY( XOR_ADD(T0, K0, X1, T1) )

STRFY( F0(X2, T0, T1) )
STRFY( ADD_XOR(T0, K1, X3, T1) )

STRFY( TEXT_SHIFT(X0, X1, X2, X3, T0) )

"SUBS R7, #1            \n\t"
"BNE  1b                \n\t"



//OFFSET SHOULD BE CORRECTED (SUB_KEY) OR OPTIMIZED VERSION
" SUB R1, R1, #4*33      \n\t"
STRFY( KEYLOAD(R1, R11, R12, M02) )
" ADD R3, R3, R11       \n\t"
" EOR R5, R5, R12       \n\t"
" AND R3, R3, R6        \n\t"

//LAST PART
//" LDR R9,  =#0X0000FFFF    \n\t"
" MOVW R9, #0XFFFF        \n\t"
" MOVT R9, #0X0000        \n\t"

" ORR R2, R2, R3, LSL #8   \n\t"//5 4 1 0
" LSR R3, R2, #16           \n\t"
" AND R2, R2, R9            \n\t"

" ORR R4, R4, R5, LSL #8   \n\t"//7 6 3 2
" LSL R5, R4, #16           \n\t"
" ORR R2, R2, R5            \n\t"
" AND R4, R4, R9, LSL#16    \n\t"
" ORR R3, R3, R4            \n\t"

" ROR R2, R2, #8            \n\t"
" AND R4, R2, #0XFF000000   \n\t"
" AND R2, R2, #0X00FFFFFF   \n\t"

" ROR R3, R3, #8            \n\t"
" AND R5, R3, #0XFF000000   \n\t"
" AND R3, R3, #0X00FFFFFF   \n\t"

" ORR R2, R2, R5            \n\t"
" ORR R3, R3, R4            \n\t"


" STM R0!, {R2-R3}        \n\t"

/*
//test
" STM R0!, {R2-R5}    \n\t"
//" STM R0!, {R8}    \n\t"
//" STM R0!, {R9}    \n\t"
//" STM R0!, {R11}    \n\t"
//" STM R0!, {R12}    \n\t"
//test
*/

STRFY(ENC_EPILOG)
/////////////////////////

  :
  : 
  : "cc", "memory"
);
}
