#define STR(...) #__VA_ARGS__
#define STRFY(...) STR(__VA_ARGS__)

#define EOL \n\t

#define CS #
#define CV(X) CS(X)

#define MASK02 #0x00FF00FF
#define MASK13 #0xFF00FF00

#define MASK_0X44 #0x01000100


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

#define DEC_PROLOG \
 PUSH {R4-R7,R14}      EOL\
 MOV R4, R8            EOL\
 MOV R5, R9            EOL\
 MOV R6, R10           EOL\
 MOV R7, R11           EOL\
 PUSH {R4-R7}          EOL\

#define DEC_EPILOG \
 POP {R4-R7}           EOL\
 MOV R8, R4            EOL\
 MOV R9, R5            EOL\
 MOV R10, R6           EOL\
 MOV R11, R7           EOL\
 POP {R4-R7,PC}        EOL\

 #define F0(X, Y, T, TT) \
    AND TT, X, MASK02      EOL\
    LSL T, TT, CV(1)        EOL\
    EOR T, T, TT, LSR CV(7) EOL\
    EOR T, T, TT, LSL CV(2) EOL\
    EOR T, T, TT, LSR CV(6) EOL\
    EOR T, T, TT, LSL CV(7) EOL\
    EOR T, T, TT, LSR CV(1) EOL\
    AND Y, T, MASK02       EOL\
    AND TT, X, MASK13      EOL\
    LSL T, TT, CV(1)        EOL\
    EOR T, T, TT, LSR CV(7) EOL\
    EOR T, T, TT, LSL CV(2) EOL\
    EOR T, T, TT, LSR CV(6) EOL\
    EOR T, T, TT, LSL CV(7) EOL\
    EOR T, T, TT, LSR CV(1) EOL\
    AND T, T, MASK13       EOL\
    ORR Y, Y, T             EOL\
    

#define F1(X, Y, T, TT) \
    AND TT, X, MASK02      EOL\
    LSL T, TT, CV(3)        EOL\
    EOR T, T, TT, LSR CV(5) EOL\
    EOR T, T, TT, LSL CV(4) EOL\
    EOR T, T, TT, LSR CV(4) EOL\
    EOR T, T, TT, LSL CV(6) EOL\
    EOR T, T, TT, LSR CV(2) EOL\
    AND Y, T, MASK02       EOL\
    AND TT, X, MASK13      EOL\
    LSL T, TT, CV(3)        EOL\
    EOR T, T, TT, LSR CV(5) EOL\
    EOR T, T, TT, LSL CV(4) EOL\
    EOR T, T, TT, LSR CV(4) EOL\
    EOR T, T, TT, LSL CV(6) EOL\
    EOR T, T, TT, LSR CV(2) EOL\
    AND T, T, MASK13        EOL\
    ORR Y, Y, T             EOL\
    

#define KEYLOAD(XP, Y0, Y1, M) \
    LDM XP!, {Y0}             EOL\
    AND Y1, Y0, M, LSL CV(8)  EOL\
    AND Y0, Y0, M             EOL\
    LSR Y1, Y1, CV(8)         EOL\

#define KEYLOAD_DEC(XP, Y0, Y1, M) \
    LDMDB XP!, {Y0}             EOL\
    AND Y1, Y0, M, LSL CV(8)  EOL\
    AND Y0, Y0, M             EOL\
    LSR Y1, Y1, CV(8)         EOL\
    

#define PSIMD_SUB(A, B)\
    USUB8 A, A, B           EOL\
/*
    ADD A, A, MASK_0X44     EOL\
    SUB A, A, B             EOL\
    AND A, A, MASK02        EOL\
*/    
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
#define XOR_SUB(X0, K0, X1, T) \
    EOR T, X0, K0          EOL\
    USUB8 X1, X1, T         EOL\
/*
    EOR T, X0, K0          EOL\
    ADD X1, X1, MASK_0X44     EOL\
    SUB X1, X1, T             EOL\
    AND X1, X1, MASK02        EOL\
*/
#define TEXT_SHIFT(X0, X1, X2, X3, T) \
    MOV T, X3, ROR CV(16)                EOL\
    MOV X3, X2               EOL\
    MOV X2, X1               EOL\
    MOV X1, X0               EOL\
    MOV X0, T                EOL\

#define TEXT_SHIFT_DEC(X0, X1, X2, X3, T) \
    MOV T, X0, ROR CV(16)                EOL\
    MOV X0, X1               EOL\
    MOV X1, X2               EOL\
    MOV X2, X3               EOL\
    MOV X3, T                EOL\

u32 __attribute__ ((noinline, naked)) dec_intra_asm(u8* input, u8* round_num, u8* random_num) {
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

STRFY(DEC_PROLOG)
/////////////////////////
" PUSH {R0}             \n\t"
" PUSH {R2}             \n\t"



" LDM R0!, {R11-R12}    \n\t"
//" SUB R0, R0, #4*2      \n\t"

//3 2 1 0
//5 4 7 6 
" MOVT R9,  #0X0000    \n\t"
" MOVW R9,  #0X00FF    \n\t"

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
/*
//SECOND MASK
" LDM R0!, {R11-R12}    \n\t"
//" SUB R0, R0, #4*4      \n\t"

" MOVT R14,  #0X0000    \n\t"
" MOVW R14,  #0X00FF    \n\t"
" AND R6, R11, R14         \n\t"
" AND R7, R11, R14, LSL#8  \n\t"
" AND R8, R11, R14, LSL#16 \n\t"
" AND R9, R11, R14, LSL#24 \n\t"

//- 4 - 0
" AND R11, R12, R14          \n\t"
" ORR R6, R6, R11, LSL#16   \n\t"

//5 - 1 -
" AND R11, R12, R14, LSL#8   \n\t"
" ORR R7, R7, R11, LSL#16   \n\t"
//- 5 - 1
" LSR R7, R7, #8            \n\t"

//- 2 - 6
" AND R11, R12, R14, LSL#16  \n\t"
" ORR R8, R8, R11, LSR#16   \n\t"
//- 6 - 2
" ROR R8, R8, #16           \n\t"

//3 - 7 -
" AND R11, R12, R14, LSL#24  \n\t"
" ORR R9, R9, R11, LSR#16   \n\t"
//- 7 - 3
" ROR R9, R9, #24           \n\t"

//combine
//4B 4A 0B 0A
" ORR R2, R2, R6, LSL#8   \n\t"
//5B 5A 1B 1A
" ORR R3, R3, R7, LSL#8   \n\t"
//6B 6A 2B 2A
" ORR R4, R4, R8, LSL#8   \n\t"
//7B 7A 3B 3A
" ORR R5, R5, R9, LSL#8   \n\t"*/
///////////////////////////////////////////////////
" ORR R2, R2, R2, LSL#8   \n\t"
" ORR R3, R3, R3, LSL#8   \n\t"
" ORR R4, R4, R4, LSL#8   \n\t"
" ORR R5, R5, R5, LSL#8   \n\t"
// 7 6 5 4 3 2 1 0
//
//" LDR R6,  =#0X00FF00FF  \n\t"//SET M02

" ADD R1, R1, #4*2      \n\t"

" LDM R1!, {R8,R9}      \n\t"
STRFY( PSIMD_SUB(R2, R8) )


" EOR R4, R4, R9        \n\t"
//" AND R2, R2, R6        \n\t"

//loading mixing
" POP {R0}             \n\t"
" LDM R0!, {R14}       \n\t"
" MOV R0, R14          \n\t"
" MOV R14, #0          \n\t"
" PUSH {R14}           \n\t"

//STRFY( TEXT_SHIFT_DEC(X0, X1, X2, X3, T0) )


" ADD R1, R1, #4*32*2      \n\t"//KEY UPDATE

" MOV R14, #32            \n\t"
"1:"
//ROUND
//STRFY( KEYLOAD_DEC(R1, K0, K1, M02) )
//"LDMDB
"LDMDB R1!, {R8, R9}  \n\t"

//mixing
" AND R10, R0, #1       \n\t"
//" MOV R10, #0 \n\t"

" LSL R10, R10, #4       \n\t"
" ROR R0, R0, #1       \n\t"
" ROR R2, R2, R10       \n\t"
" ROR R3, R3, R10       \n\t"
" ROR R4, R4, R10       \n\t"
" ROR R5, R5, R10       \n\t"

" POP {R11}             \n\t"
" EOR R10, R10, R11     \n\t"
" PUSH {R10}             \n\t"

" ROR R8, R8, R10       \n\t"
" ROR R9, R9, R10       \n\t"




STRFY( F1(X0, T0, T1, T2) )
STRFY( XOR_SUB(T0, K0, X1, T1) )

STRFY( F0(X2, T0, T1, T2) )
STRFY( ADD_XOR(T0, K1, X3, T1) )

STRFY( TEXT_SHIFT_DEC(X0, X1, X2, X3, T0) )

"SUBS R14, #1            \n\t"
"BNE  1b                \n\t"




//OFFSET SHOULD BE CORRECTED (SUB_KEY) OR OPTIMIZED VERSION
" SUB R1, R1, #4*1*2      \n\t"
//STRFY( KEYLOAD_DEC(R1, R11, R12, M02) )
"LDMDB R1!, {R11,R12}  \n\t"



" POP {R14}             \n\t"


" ROR R2, R2, R14       \n\t"
" ROR R3, R3, R14       \n\t"
" ROR R4, R4, R14       \n\t"
" ROR R5, R5, R14       \n\t"




STRFY( TEXT_SHIFT(X0, X1, X2, X3, T0) )
//" ADD R3, R3, R11       \n\t"
STRFY( PSIMD_SUB(R2, R11) )

" EOR R4, R4, R12       \n\t"





///////////////////////////////////////////////////
//LAST PART
" AND R6, R2, #0xFF00FF00   \n\t"
" AND R7, R3, #0xFF00FF00   \n\t"
" AND R8, R4, #0xFF00FF00   \n\t"
" AND R9, R5, #0xFF00FF00   \n\t"

" AND R2, R2, #0x00FF00FF   \n\t"
" AND R3, R3, #0x00FF00FF   \n\t"
" AND R4, R4, #0x00FF00FF   \n\t"
" AND R5, R5, #0x00FF00FF   \n\t"


" EOR R10, R6, R2, LSL #8   \n\t"
" EOR R11, R7, R3, LSL #8   \n\t"
" EOR R12, R8, R4, LSL #8   \n\t"
" EOR R14, R9, R5, LSL #8   \n\t"

" ADD R10, R10, R11     \n\t"
" ADD R12, R12, R14     \n\t"
" ADD R10, R10, R12     \n\t"

//
" MOVT R14,  #0X0000    \n\t"
" MOVW R14,  #0XFFFF    \n\t"
" ORR R2, R2, R3, LSL #8   \n\t"//5 4 1 0
" LSR R3, R2, #16           \n\t"
" AND R2, R2, R14            \n\t"

" ORR R4, R4, R5, LSL #8   \n\t"//7 6 3 2
" LSL R5, R4, #16           \n\t"
" ORR R2, R2, R5            \n\t"
" AND R4, R4, R14, LSL#16    \n\t"
" ORR R3, R3, R4            \n\t"


/*
" ROR R2, R2, #8            \n\t"
" AND R4, R2, #0XFF000000   \n\t"
" AND R2, R2, #0X00FFFFFF   \n\t"

" ROR R3, R3, #8            \n\t"
" AND R5, R3, #0XFF000000   \n\t"
" AND R3, R3, #0X00FFFFFF   \n\t"

" ORR R2, R2, R5            \n\t"
" ORR R3, R3, R4            \n\t"


" ROR R2, R2, #24           \n\t"
" AND R10, R2, #0X000000FF   \n\t"
" AND R2, R2, #0XFFFFFF00   \n\t"

" ROR R3, R3, #24            \n\t"
" AND R11, R3, #0X000000FF   \n\t"
" AND R3, R3, #0XFFFFFF00   \n\t"
" ORR R2, R2, R11 \n\t"
" ORR R3, R3, R10 \n\t"
*/
" POP {R0}                \n\t"
" STM R0!, {R2-R3}        \n\t"


" MOV R0, R10 \n\t"

/*
//
" LSR R2, R6, #8           \n\t"
" LSR R3, R7, #8           \n\t"
" LSR R4, R8, #8           \n\t"
" LSR R5, R9, #8           \n\t"

" MOVT R14,  #0X0000    \n\t"
" MOVW R14,  #0XFFFF    \n\t"
" ORR R2, R2, R3, LSL #8   \n\t"//5 4 1 0
" LSR R3, R2, #16           \n\t"
" AND R2, R2, R14            \n\t"

" ORR R4, R4, R5, LSL #8   \n\t"//7 6 3 2
" LSL R5, R4, #16           \n\t"
" ORR R2, R2, R5            \n\t"
" AND R4, R4, R14, LSL#16    \n\t"
" ORR R3, R3, R4            \n\t"
*/
/*
" ROR R2, R2, #8            \n\t"
" AND R4, R2, #0XFF000000   \n\t"
" AND R2, R2, #0X00FFFFFF   \n\t"

" ROR R3, R3, #8            \n\t"
" AND R5, R3, #0XFF000000   \n\t"
" AND R3, R3, #0X00FFFFFF   \n\t"

" ORR R2, R2, R5            \n\t"
" ORR R3, R3, R4            \n\t"
*/
//" STM R0!, {R2-R3}        \n\t"
STRFY(DEC_EPILOG)
/////////////////////////

  :
  : 
  : "cc", "memory"
);
}
