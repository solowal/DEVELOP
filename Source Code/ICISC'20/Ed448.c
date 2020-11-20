#include <stdio.h>
#include <stdint.h>
#include <string.h>

//
typedef unsigned long u32;
typedef unsigned char u8;
#define get_bit(k, i) (( k[ (i) >> 5 ] >> ((i) & 0x1F)) & 1)

typedef struct point448
{
	u32 x[14];
	u32 y[14];
	u32 z[14];
	u32 e[14];
	u32 h[14];
	u32 proj;
} POINT448;

void PointAdd448(POINT448 *R, POINT448 *P, POINT448 *Q);
void PointDbl448(POINT448 *R, POINT448 *P);
void ProToAff448(POINT448* R, POINT448* P);
void PointXZMulSecure448(POINT448* R, u32* k, POINT448* P);

void	fp_add(u32* c, u32* a, u32* b);
void	fp_sub(u32* c, u32* a, u32* b);
void	fp_mul(u32* c, u32* a, u32* b) ;
void	fp_sqr(u32* c, u32* a);
void	fp_inv(u32* c, u32* z);
//

#define STR(...) #__VA_ARGS__
#define STRFY(...) STR(__VA_ARGS__)
#define EOL \n\t
#define CS #
#define CV(X) CS(X)

#define P_TMP   R1

#define P_OP_A0 R2
#define P_OP_A1 R3
#define P_OP_A2 R4
#define P_OP_A3 R5

#define P_OP_B0 R6
#define P_OP_B1 R7
#define P_OP_B2 R8
#define P_OP_B3 R9

#define P_RST0 R10
#define P_RST1 R11
#define P_RST2 R12
#define P_RST3 R14


#define T0   R0
#define T1   R1
#define T2   R2
#define T3   R3
#define T4   R4
#define T5   R5
#define T6   R6

#define A0   	R7
#define A1   	R8
#define A2   	R9
#define A3   	R10
#define A4   	R11
#define A5   	R12
#define A6   	R14

#define P_MUL_PROLOG \
 PUSH {R0,R1,R2,R4-R11,R14}      EOL\

#define P_MUL_EPILOG \
 POP {R4-R11,PC}        EOL\

#define P_LOAD(OP, V0, V1, V2, V3, OFFSET) \
    LDR V0, [OP, CV(4*OFFSET)]            EOL\
    LDR V1, [OP, CV(4*OFFSET+4)]          EOL\
    LDR V2, [OP, CV(4*OFFSET+8)]          EOL\
    LDR V3, [OP, CV(4*OFFSET+12)]         EOL\

#define P_LOAD2(OP, V0, V1, OFFSET) \
    LDR V0, [OP, CV(4*OFFSET)]            EOL\
    LDR V1, [OP, CV(4*OFFSET+4)]          EOL\

#define P_MUL_TOP(R_OUT, OFFSET) \
	UMULL P_TMP, P_RST0, P_OP_A0, P_OP_B0         EOL\
    STR P_TMP, [R_OUT, CV(4*OFFSET + 0)]    EOL\
    MOV P_TMP, CV(0)                        EOL\
    UMAAL P_TMP, P_RST0, P_OP_A1, P_OP_B0         EOL\
    MOV P_RST1, CV(0)                       EOL\
    UMAAL P_TMP, P_RST1, P_OP_A0, P_OP_B1         EOL\
    STR P_TMP, [R_OUT, CV(4*OFFSET + 4)]    EOL\
	UMAAL P_RST0, P_RST1, P_OP_A1, P_OP_B1        EOL\
    STR P_RST0, [R_OUT, CV(4*OFFSET + 8)]   EOL\
    STR P_RST1, [R_OUT, CV(4*OFFSET + 12)]  EOL\
	
#define P_MUL_FRONT(R_OUT, OFFSET) \
    UMULL P_TMP, P_RST0, P_OP_A0, P_OP_B0         EOL\
    STR P_TMP, [R_OUT, CV(4*OFFSET + 0)]    EOL\
    UMULL P_TMP, P_RST1, P_OP_A1, P_OP_B0         EOL\
    UMAAL P_TMP, P_RST0, P_OP_A0, P_OP_B1         EOL\
    STR P_TMP, [R_OUT, CV(4*OFFSET + 4)]    EOL\
    UMULL P_TMP, P_RST2, P_OP_A2, P_OP_B0         EOL\
    UMAAL P_TMP, P_RST1, P_OP_A1, P_OP_B1         EOL\
    UMAAL P_TMP, P_RST0, P_OP_A0, P_OP_B2         EOL\
    STR P_TMP, [R_OUT, CV(4*OFFSET + 8)]    EOL\
    UMULL P_TMP, P_RST3, P_OP_A0, P_OP_B3         EOL\
    UMAAL P_TMP, P_RST0, P_OP_A3, P_OP_B0         EOL\
    UMAAL P_TMP, P_RST1, P_OP_A2, P_OP_B1         EOL\
    UMAAL P_TMP, P_RST2, P_OP_A1, P_OP_B2         EOL\
    STR P_TMP, [R_OUT, CV(4*OFFSET + 12)]   EOL\
    
#define P_MUL_BACK(R_OUT, OFFSET) \
    UMAAL P_RST0, P_RST1, P_OP_A3, P_OP_B1        EOL\
    UMAAL P_RST0, P_RST2, P_OP_A2, P_OP_B2        EOL\
    UMAAL P_RST0, P_RST3, P_OP_A1, P_OP_B3        EOL\
    STR P_RST0, [R_OUT, CV(4*OFFSET + 0)]   EOL\
    UMAAL P_RST1, P_RST2, P_OP_A3, P_OP_B2        EOL\
    UMAAL P_RST1, P_RST3, P_OP_A2, P_OP_B3        EOL\
    STR P_RST1, [R_OUT, CV(4*OFFSET + 4)]   EOL\
    UMAAL P_RST2, P_RST3, P_OP_A3, P_OP_B3        EOL\
    STR P_RST2, [R_OUT, CV(4*OFFSET + 8)]   EOL\
    STR P_RST3, [R_OUT, CV(4*OFFSET + 12)]  EOL\

#define P_MUL_BACK2(R_OUT, OFFSET) \
    UMAAL P_RST0, P_RST1, P_OP_A1, P_OP_B3        EOL\
    UMAAL P_RST0, P_RST2, P_OP_A0, P_OP_B0        EOL\
    UMAAL P_RST0, P_RST3, P_OP_A3, P_OP_B1        EOL\
    STR P_RST0, [R_OUT, CV(4*OFFSET + 0)]   EOL\
    UMAAL P_RST1, P_RST2, P_OP_A1, P_OP_B0        EOL\
    UMAAL P_RST1, P_RST3, P_OP_A0, P_OP_B1        EOL\
    STR P_RST1, [R_OUT, CV(4*OFFSET + 4)]   EOL\
    UMAAL P_RST2, P_RST3, P_OP_A1, P_OP_B1        EOL\
    STR P_RST2, [R_OUT, CV(4*OFFSET + 8)]   EOL\
    STR P_RST3, [R_OUT, CV(4*OFFSET + 12)]  EOL\

#define P_MUL_MID_OP_B_SHORT(R_OUT, R_OFF, OP_P, P_OFF) \
    LDR P_OP_B0, [OP_P, CV(4*P_OFF + 0)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 0)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A3, P_OP_B1        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A2, P_OP_B2        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A1, P_OP_B3        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A0, P_OP_B0        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 0)]     EOL\
    LDR P_OP_B1, [OP_P, CV(4*P_OFF + 4)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 4)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A3, P_OP_B2        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A2, P_OP_B3        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A1, P_OP_B0        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A0, P_OP_B1        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 4)]     EOL\
    

#define P_MUL_MID_OP_B(R_OUT, R_OFF, OP_P, P_OFF) \
    LDR P_OP_B0, [OP_P, CV(4*P_OFF + 0)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 0)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A3, P_OP_B1        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A2, P_OP_B2        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A1, P_OP_B3        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A0, P_OP_B0        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 0)]     EOL\
    LDR P_OP_B1, [OP_P, CV(4*P_OFF + 4)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 4)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A3, P_OP_B2        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A2, P_OP_B3        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A1, P_OP_B0        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A0, P_OP_B1        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 4)]     EOL\
    LDR P_OP_B2, [OP_P, CV(4*P_OFF + 8)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 8)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A3, P_OP_B3        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A2, P_OP_B0        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A1, P_OP_B1        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A0, P_OP_B2        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 8)]     EOL\
    LDR P_OP_B3, [OP_P, CV(4*P_OFF + 12)]   EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 12)]    EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A3, P_OP_B0        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A2, P_OP_B1        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A1, P_OP_B2        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A0, P_OP_B3        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 12)]    EOL\

#define P_MUL_MID_OP_A_SHORT(R_OUT, R_OFF, OP_P, P_OFF) \
    LDR P_OP_A0, [OP_P, CV(4*P_OFF + 0)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 0)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A0, P_OP_B2        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A3, P_OP_B3        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A2, P_OP_B0        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A1, P_OP_B1        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 0)]     EOL\
    LDR P_OP_A1, [OP_P, CV(4*P_OFF + 4)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 4)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A1, P_OP_B2        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A0, P_OP_B3        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A3, P_OP_B0        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A2, P_OP_B1        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 4)]     EOL\

#define P_MUL_MID_OP_A(R_OUT, R_OFF, OP_P, P_OFF) \
    LDR P_OP_A0, [OP_P, CV(4*P_OFF + 0)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 0)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A0, P_OP_B0        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A3, P_OP_B1        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A2, P_OP_B2        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A1, P_OP_B3        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 0)]     EOL\
    LDR P_OP_A1, [OP_P, CV(4*P_OFF + 4)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 4)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A1, P_OP_B0        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A0, P_OP_B1        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A3, P_OP_B2        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A2, P_OP_B3        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 4)]     EOL\
    LDR P_OP_A2, [OP_P, CV(4*P_OFF + 8)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 8)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A2, P_OP_B0        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A1, P_OP_B1        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A0, P_OP_B2        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A3, P_OP_B3        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 8)]     EOL\
    LDR P_OP_A3, [OP_P, CV(4*P_OFF + 12)]   EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 12)]    EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A3, P_OP_B0        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A2, P_OP_B1        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A1, P_OP_B2        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A0, P_OP_B3        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 12)]    EOL\

#define P_MUL_MID_OP_A2(R_OUT, R_OFF, OP_P, P_OFF) \
    LDR P_OP_A2, [OP_P, CV(4*P_OFF + 0)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 0)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A2, P_OP_B2        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A1, P_OP_B3        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A0, P_OP_B0        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A3, P_OP_B1        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 0)]     EOL\
    LDR P_OP_A3, [OP_P, CV(4*P_OFF + 4)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 4)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A3, P_OP_B2        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A2, P_OP_B3        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A1, P_OP_B0        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A0, P_OP_B1        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 4)]     EOL\
    LDR P_OP_A0, [OP_P, CV(4*P_OFF + 8)]    EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 8)]     EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A0, P_OP_B2        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A3, P_OP_B3        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A2, P_OP_B0        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A1, P_OP_B1        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 8)]     EOL\
    LDR P_OP_A1, [OP_P, CV(4*P_OFF + 12)]   EOL\
    LDR P_TMP, [R_OUT, CV(4*R_OFF + 12)]    EOL\
    UMAAL P_RST3,  P_RST0, P_OP_A1, P_OP_B2        EOL\
    UMAAL P_RST3,  P_RST1, P_OP_A0, P_OP_B3        EOL\
    UMAAL P_RST3,  P_RST2, P_OP_A3, P_OP_B0        EOL\
    UMAAL P_TMP,  P_RST3, P_OP_A2, P_OP_B1        EOL\
    STR P_TMP, [R_OUT, CV(4*R_OFF + 12)]    EOL\

#define RED(P_OFF) \
	MOV A0, CV(0)			EOL\
	/*e0||T = A[2] + A[3]*/	EOL\
	LDR T0, [SP, CV(4 * 14)]	EOL\
	LDR T1, [SP, CV(4 * 15)]	EOL\
	LDR T2, [SP, CV(4 * 16)]	EOL\
	LDR T3, [SP, CV(4 * 17)]	EOL\
	LDR T4, [SP, CV(4 * 18)]	EOL\
	LDR T5, [SP, CV(4 * 19)]	EOL\
	LDR T6, [SP, CV(4 * 20)]	EOL\
	LDR A0, [SP, CV(4 * 21)]	EOL\
	LDR A1, [SP, CV(4 * 22)]	EOL\
	LDR A2, [SP, CV(4 * 23)]	EOL\
	LDR A3, [SP, CV(4 * 24)]	EOL\
	LDR A4, [SP, CV(4 * 25)]	EOL\
	LDR A5, [SP, CV(4 * 26)]	EOL\
	LDR A6, [SP, CV(4 * 27)]	EOL\
	ADDS T0, T0, A0			EOL\
	ADCS T1, T1, A1			EOL\
	ADCS T2, T2, A2			EOL\
	ADCS T3, T3, A3			EOL\
	ADCS T4, T4, A4			EOL\
	ADCS T5, T5, A5			EOL\
	ADCS T6, T6, A6			EOL\
	MOV	 A0, CV(0)			EOL\
	ADCS A0, A0, CV(0)		EOL\
	STR A0, [SP, CV(4 * 14)]	/* E0 */	EOL\
	/*e1||C[0] = A[0] + e0||T*/	EOL\
	LDR A0, [SP, CV(4 * 0)]	EOL\
	LDR A1, [SP, CV(4 * 1)]	EOL\
	LDR A2, [SP, CV(4 * 2)]	EOL\
	LDR A3, [SP, CV(4 * 3)]	EOL\
	LDR A4, [SP, CV(4 * 4)]	EOL\
	LDR A5, [SP, CV(4 * 5)]	EOL\
	LDR A6, [SP, CV(4 * 6)]	EOL\
	ADDS A0, T0, A0			EOL\
	ADCS A1, T1, A1			EOL\
	ADCS A2, T2, A2			EOL\
	ADCS A3, T3, A3			EOL\
	ADCS A4, T4, A4			EOL\
	ADCS A5, T5, A5			EOL\
	ADCS A6, T6, A6			EOL\
	STR A0, [SP, CV(4 * 0)]	EOL\
	STR A1, [SP, CV(4 * 1)]	EOL\
	STR A2, [SP, CV(4 * 2)]	EOL\
	STR A3, [SP, CV(4 * 3)]	EOL\
	STR A4, [SP, CV(4 * 4)]	EOL\
	STR A5, [SP, CV(4 * 5)]	EOL\
	STR A6, [SP, CV(4 * 6)]	EOL\
	LDR A0, [SP, CV(4 * 14)]	EOL\
	ADCS A0, A0, CV(0)		EOL\
	STR A0, [SP, CV(4 * 15)]	/* E1 */	EOL\
	/*e2||C[1] = A[1] + A[3] + e0||T*/	EOL\
	LDR A0, [SP, CV(4 * 7)]	EOL\
	LDR A1, [SP, CV(4 * 8)]	EOL\
	LDR A2, [SP, CV(4 * 9)]	EOL\
	LDR A3, [SP, CV(4 * 10)]	EOL\
	LDR A4, [SP, CV(4 * 11)]	EOL\
	LDR A5, [SP, CV(4 * 12)]	EOL\
	LDR A6, [SP, CV(4 * 13)]	EOL\
	ADDS A0, T0, A0			EOL\
	ADCS A1, T1, A1			EOL\
	ADCS A2, T2, A2			EOL\
	ADCS A3, T3, A3			EOL\
	ADCS A4, T4, A4			EOL\
	ADCS A5, T5, A5			EOL\
	ADCS A6, T6, A6			EOL\
	STR A0, [SP, CV(4 * 7)]	EOL\
	LDR A0, [SP, CV(4 * 14)]	EOL\
	ADCS A0, A0, CV(0)		EOL\
	STR A0, [SP, CV(4 * 16)]	/* E2 */	EOL\
	LDR A0, [SP, CV(4 * 7)]	EOL\
	LDR T0, [SP, CV(4 * 21)]	EOL\
	LDR T1, [SP, CV(4 * 22)]	EOL\
	LDR T2, [SP, CV(4 * 23)]	EOL\
	LDR T3, [SP, CV(4 * 24)]	EOL\
	LDR T4, [SP, CV(4 * 25)]	EOL\
	LDR T5, [SP, CV(4 * 26)]	EOL\
	LDR T6, [SP, CV(4 * 27)]	EOL\
	ADDS A0, T0, A0			EOL\
	ADCS A1, T1, A1			EOL\
	ADCS A2, T2, A2			EOL\
	ADCS A3, T3, A3			EOL\
	ADCS A4, T4, A4			EOL\
	ADCS A5, T5, A5			EOL\
	ADCS A6, T6, A6			EOL\
	STR A0, [SP, CV(4 * 7)]	EOL\
	LDR A0, [SP, CV(4 * 16)]	/* E2 */	EOL\
	ADCS A0, A0, CV(0)		EOL\
	STR A0, [SP, CV(4 * 16)]	/* E2 */	EOL\
	/*e3||C[0] = C[0] + e2*/	EOL\
	LDR T0, [SP, CV(4 * 0)]	EOL\
	LDR T1, [SP, CV(4 * 1)]	EOL\
	LDR T2, [SP, CV(4 * 2)]	EOL\
	LDR T3, [SP, CV(4 * 3)]	EOL\
	LDR T4, [SP, CV(4 * 4)]	EOL\
	LDR T5, [SP, CV(4 * 5)]	EOL\
	LDR T6, [SP, CV(4 * 6)]	EOL\
	ADDS T0, T0, A0			EOL\
	ADCS T1, T1, CV(0)		EOL\
	ADCS T2, T2, CV(0)		EOL\
	ADCS T3, T3, CV(0)		EOL\
	ADCS T4, T4, CV(0)		EOL\
	ADCS T5, T5, CV(0)		EOL\
	ADCS T6, T6, CV(0)		EOL\
	STR T0, [SP, CV(4 * 0)]	EOL\
	/*e4||C[1] = C[1] + e1 + e2 + e3*/	EOL\
	MOV T0, CV(0)			EOL\
	ADCS T0, T0, CV(0)		/* E3 */	EOL\
	ADD T0, T0, A0			/* E2 + E3 */ EOL\
	LDR A0, [SP, CV(4 * 15)]	EOL\
	ADD T0, T0, A0			/* E1 + E2 + E3 */ EOL\
	LDR A0, [SP, CV(4 * 7)]	EOL\
	ADDS A0, A0, T0			EOL\
	ADCS A1, A1, CV(0)		EOL\
	ADCS A2, A2, CV(0)		EOL\
	ADCS A3, A3, CV(0)		EOL\
	ADCS A4, A4, CV(0)		EOL\
	ADCS A5, A5, CV(0)		EOL\
	ADCS A6, A6, CV(0)		EOL\
	STR A0, [SP, CV(4 * 7)]	EOL\
	MOV A0, CV(0)			EOL\
	ADCS A0, A0, CV(0)		/* E4 */	EOL\
	/*e5||C[0] = C[0] + e4*/	EOL\
	STR A1, [SP, CV(4 * 8)]	EOL\
	LDR A1, [SP, CV(4 * P_OFF)] /* RESULT POINTER*/ EOL\
	LDR T0, [SP, CV(4 * 0)]	EOL\
	ADDS T0, T0, A0			EOL\
	ADCS T1, T1, CV(0)		EOL\
	ADCS T2, T2, CV(0)		EOL\
	ADCS T3, T3, CV(0)		EOL\
	ADCS T4, T4, CV(0)		EOL\
	ADCS T5, T5, CV(0)		EOL\
	ADCS T6, T6, CV(0)		EOL\
	STR T0, [A1, CV(4 * 0)]	EOL\
	STR T1, [A1, CV(4 * 1)]	EOL\
	STR T2, [A1, CV(4 * 2)]	EOL\
	STR T3, [A1, CV(4 * 3)]	EOL\
	STR T4, [A1, CV(4 * 4)]	EOL\
	STR T5, [A1, CV(4 * 5)]	EOL\
	STR T6, [A1, CV(4 * 6)]	EOL\
	ADCS T0, A0, CV(0) /* E4 + E5*/ EOL\
	MOV T1, A1				EOL\
	LDR A0, [SP, CV(4 * 7)]	EOL\
	LDR A1, [SP, CV(4 * 8)]	EOL\
	/*C[1] = C[1] + (e4+e5) */ EOL\
	ADDS A0, A0, T0			EOL\
	ADCS A1, A1, CV(0)		EOL\
	ADCS A2, A2, CV(0)		EOL\
	ADCS A3, A3, CV(0)		EOL\
	ADCS A4, A4, CV(0)		EOL\
	ADCS A5, A5, CV(0)		EOL\
	ADCS A6, A6, CV(0)		EOL\
	STR A0, [T1, CV(4 * 7)]	EOL\
	STR A1, [T1, CV(4 * 8)]	EOL\
	STR A2, [T1, CV(4 * 9)]	EOL\
	STR A3, [T1, CV(4 * 10)]	EOL\
	STR A4, [T1, CV(4 * 11)]	EOL\
	STR A5, [T1, CV(4 * 12)]	EOL\
	STR A6, [T1, CV(4 * 13)]	EOL\
	
#define Q0 R3
#define Q1 R4
#define Q2 R5
#define Q3 R6
#define Q4 R7
#define Q5 R8
#define Q6 R9
#define Q7 R10
#define Q8 R11
#define TT0 R12
#define TT1 R14

#define ADD_ASM \
	LDR Q0, [R0, CV(4 * 0)]			EOL\
	LDR Q1, [R0, CV(4 * 1)]			EOL\
	LDR Q2, [R0, CV(4 * 2)]			EOL\
	LDR Q3, [R0, CV(4 * 3)]			EOL\
	LDR Q4, [R0, CV(4 * 4)]			EOL\
	LDR Q5, [R0, CV(4 * 5)]			EOL\
	LDR Q6, [R0, CV(4 * 6)]			EOL\
	LDR Q7, [R0, CV(4 * 7)]			EOL\
	LDR Q8, [R0, CV(4 * 8)]			EOL\
	LDR TT1, [R1, CV(4 * 0)]			EOL\
	ADDS Q0, Q0, TT1				EOL\
	LDR TT1, [R1, CV(4 * 1)]			EOL\
	ADCS Q1, Q1, TT1				EOL\
	LDR TT1, [R1, CV(4 * 2)]			EOL\
	ADCS Q2, Q2, TT1				EOL\
	LDR TT1, [R1, CV(4 * 3)]			EOL\
	ADCS Q3, Q3, TT1				EOL\
	LDR TT1, [R1, CV(4 * 4)]			EOL\
	ADCS Q4, Q4, TT1				EOL\
	LDR TT1, [R1, CV(4 * 5)]			EOL\
	ADCS Q5, Q5, TT1				EOL\
	LDR TT1, [R1, CV(4 * 6)]			EOL\
	ADCS Q6, Q6, TT1				EOL\
	LDR TT1, [R1, CV(4 * 7)]			EOL\
	ADCS Q7, Q7, TT1				EOL\
	LDR TT1, [R1, CV(4 * 8)]			EOL\
	ADCS Q8, Q8, TT1				EOL\
	LDR TT0, [R0, CV(4 * 9)]			EOL\
	LDR TT1, [R1, CV(4 * 9)]			EOL\
	ADCS TT0, TT0, TT1				EOL\
	STR TT0, [SP, CV(4 * 9)]			EOL\
	LDR TT0, [R0, CV(4 * 10)]			EOL\
	LDR TT1, [R1, CV(4 * 10)]			EOL\
	ADCS TT0, TT0, TT1				EOL\
	STR TT0, [SP, CV(4 * 10)]			EOL\
	LDR TT0, [R0, CV(4 * 11)]			EOL\
	LDR TT1, [R1, CV(4 * 11)]			EOL\
	ADCS TT0, TT0, TT1				EOL\
	STR TT0, [SP, CV(4 * 11)]			EOL\
	LDR TT0, [R0, CV(4 * 12)]			EOL\
	LDR TT1, [R1, CV(4 * 12)]			EOL\
	ADCS TT0, TT0, TT1				EOL\
	STR TT0, [SP, CV(4 * 12)]			EOL\
	LDR TT0, [R0, CV(4 * 13)]			EOL\
	LDR TT1, [R1, CV(4 * 13)]			EOL\
	ADCS TT0, TT0, TT1				EOL\
	STR TT0, [SP, CV(4 * 13)]			EOL\
	MOV TT1, CV(0)					EOL\
	ADCS TT0, TT1, CV(0)			EOL\
	SUB  TT1, TT1, TT0		/*FF*/		EOL\
	SUB  TT0, TT1, TT0		/*FE*/		EOL\
	SUBS Q0, Q0, TT1				EOL\
	SBCS Q1, Q1, TT1				EOL\
	SBCS Q2, Q2, TT1				EOL\
	SBCS Q3, Q3, TT1				EOL\
	SBCS Q4, Q4, TT1				EOL\
	SBCS Q5, Q5, TT1				EOL\
	SBCS Q6, Q6, TT1				EOL\
	SBCS Q7, Q7, TT0				EOL\
	SBCS Q8, Q8, TT1				EOL\
	STR Q0, [R2, CV(4 * 0)]			EOL\
	STR Q1, [R2, CV(4 * 1)]			EOL\
	STR Q2, [R2, CV(4 * 2)]			EOL\
	STR Q3, [R2, CV(4 * 3)]			EOL\
	STR Q4, [R2, CV(4 * 4)]			EOL\
	STR Q5, [R2, CV(4 * 5)]			EOL\
	STR Q6, [R2, CV(4 * 6)]			EOL\
	STR Q7, [R2, CV(4 * 7)]			EOL\
	STR Q8, [R2, CV(4 * 8)]			EOL\
	LDR Q0, [SP, CV(4 * 9)]			EOL\
	LDR Q1, [SP, CV(4 * 10)]			EOL\
	LDR Q2, [SP, CV(4 * 11)]			EOL\
	LDR Q3, [SP, CV(4 * 12)]			EOL\
	LDR Q4, [SP, CV(4 * 13)]			EOL\
	SBCS Q0, Q0, TT1				EOL\
	SBCS Q1, Q1, TT1				EOL\
	SBCS Q2, Q2, TT1				EOL\
	SBCS Q3, Q3, TT1				EOL\
	SBCS Q4, Q4, TT1				EOL\
	STR Q0, [R2, CV(4 * 9)]			EOL\
	STR Q1, [R2, CV(4 * 10)]			EOL\
	STR Q2, [R2, CV(4 * 11)]			EOL\
	STR Q3, [R2, CV(4 * 12)]			EOL\
	STR Q4, [R2, CV(4 * 13)]			EOL\
	
#define SUB_ASM \
	LDR Q0, [R0, CV(4 * 0)]			EOL\
	LDR Q1, [R0, CV(4 * 1)]			EOL\
	LDR Q2, [R0, CV(4 * 2)]			EOL\
	LDR Q3, [R0, CV(4 * 3)]			EOL\
	LDR Q4, [R0, CV(4 * 4)]			EOL\
	LDR Q5, [R0, CV(4 * 5)]			EOL\
	LDR Q6, [R0, CV(4 * 6)]			EOL\
	LDR Q7, [R0, CV(4 * 7)]			EOL\
	LDR Q8, [R0, CV(4 * 8)]			EOL\
	LDR TT1, [R1, CV(4 * 0)]			EOL\
	SUBS Q0, Q0, TT1				EOL\
	LDR TT1, [R1, CV(4 * 1)]			EOL\
	SBCS Q1, Q1, TT1				EOL\
	LDR TT1, [R1, CV(4 * 2)]			EOL\
	SBCS Q2, Q2, TT1				EOL\
	LDR TT1, [R1, CV(4 * 3)]			EOL\
	SBCS Q3, Q3, TT1				EOL\
	LDR TT1, [R1, CV(4 * 4)]			EOL\
	SBCS Q4, Q4, TT1				EOL\
	LDR TT1, [R1, CV(4 * 5)]			EOL\
	SBCS Q5, Q5, TT1				EOL\
	LDR TT1, [R1, CV(4 * 6)]			EOL\
	SBCS Q6, Q6, TT1				EOL\
	LDR TT1, [R1, CV(4 * 7)]			EOL\
	SBCS Q7, Q7, TT1				EOL\
	LDR TT1, [R1, CV(4 * 8)]			EOL\
	SBCS Q8, Q8, TT1				EOL\
	LDR TT0, [R0, CV(4 * 9)]			EOL\
	LDR TT1, [R1, CV(4 * 9)]			EOL\
	SBCS TT0, TT0, TT1				EOL\
	STR TT0, [SP, CV(4 * 9)]			EOL\
	LDR TT0, [R0, CV(4 * 10)]			EOL\
	LDR TT1, [R1, CV(4 * 10)]			EOL\
	SBCS TT0, TT0, TT1				EOL\
	STR TT0, [SP, CV(4 * 10)]			EOL\
	LDR TT0, [R0, CV(4 * 11)]			EOL\
	LDR TT1, [R1, CV(4 * 11)]			EOL\
	SBCS TT0, TT0, TT1				EOL\
	STR TT0, [SP, CV(4 * 11)]			EOL\
	LDR TT0, [R0, CV(4 * 12)]			EOL\
	LDR TT1, [R1, CV(4 * 12)]			EOL\
	SBCS TT0, TT0, TT1				EOL\
	STR TT0, [SP, CV(4 * 12)]			EOL\
	LDR TT0, [R0, CV(4 * 13)]			EOL\
	LDR TT1, [R1, CV(4 * 13)]			EOL\
	SBCS TT0, TT0, TT1				EOL\
	STR TT0, [SP, CV(4 * 13)]			EOL\
	SBCS TT1, TT1, TT1		/*FF*/	EOL\
	ADD  TT0, TT1, TT1		/*FE*/		EOL\
	ADDS Q0, Q0, TT1				EOL\
	ADCS Q1, Q1, TT1				EOL\
	ADCS Q2, Q2, TT1				EOL\
	ADCS Q3, Q3, TT1				EOL\
	ADCS Q4, Q4, TT1				EOL\
	ADCS Q5, Q5, TT1				EOL\
	ADCS Q6, Q6, TT1				EOL\
	ADCS Q7, Q7, TT0				EOL\
	ADCS Q8, Q8, TT1				EOL\
	STR Q0, [R2, CV(4 * 0)]			EOL\
	STR Q1, [R2, CV(4 * 1)]			EOL\
	STR Q2, [R2, CV(4 * 2)]			EOL\
	STR Q3, [R2, CV(4 * 3)]			EOL\
	STR Q4, [R2, CV(4 * 4)]			EOL\
	STR Q5, [R2, CV(4 * 5)]			EOL\
	STR Q6, [R2, CV(4 * 6)]			EOL\
	STR Q7, [R2, CV(4 * 7)]			EOL\
	STR Q8, [R2, CV(4 * 8)]			EOL\
	LDR Q0, [SP, CV(4 * 9)]			EOL\
	LDR Q1, [SP, CV(4 * 10)]			EOL\
	LDR Q2, [SP, CV(4 * 11)]			EOL\
	LDR Q3, [SP, CV(4 * 12)]			EOL\
	LDR Q4, [SP, CV(4 * 13)]			EOL\
	ADCS Q0, Q0, TT1				EOL\
	ADCS Q1, Q1, TT1				EOL\
	ADCS Q2, Q2, TT1				EOL\
	ADCS Q3, Q3, TT1				EOL\
	ADCS Q4, Q4, TT1				EOL\
	STR Q0, [R2, CV(4 * 9)]			EOL\
	STR Q1, [R2, CV(4 * 10)]			EOL\
	STR Q2, [R2, CV(4 * 11)]			EOL\
	STR Q3, [R2, CV(4 * 12)]			EOL\
	STR Q4, [R2, CV(4 * 13)]			EOL\


static u32 t1[14];

static u32 d[14] = {0xFFFF6755,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFE,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
static u32 one_div_2[14] = {0x0,0x0,0x0,0x0,0x0,0x0,0x80000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x7FFFFFFF};

static POINT448   T[2];
static POINT448   Q;
static 	u32 z3[14*2]={0,}, t0[14*2]={0,},  t2[14*2]={0,};


void  __attribute__ ((noinline, naked)) fp_mul434(u32* a, u32* b, u32* c) {
asm volatile(\
STRFY(P_MUL_PROLOG)
"SUB SP, #4*28 			\n\t"

//ROUND#1
//"LDR R0, [SP, #4 * 28] \n\t"// OP_A
//"LDR R1, [SP, #4 * 29] \n\t"// OP_B
STRFY(P_LOAD2(R0, P_OP_A0, P_OP_A1, 12))
STRFY(P_LOAD(R1, P_OP_B0, P_OP_B1, P_OP_B2, P_OP_B3, 0))
STRFY(P_MUL_TOP(SP, 12))

//ROUND#2
//"LDR R0, [SP, #4 * 28] \n\t"// OP_A
//"LDR R1, [SP, #4 * 29] \n\t"// OP_B
STRFY(P_LOAD(R0, P_OP_A0, P_OP_A1, P_OP_A2, P_OP_A3, 8))
//LOAD(R1, OP_B0, OP_B1, OP_B2, OP_B3, 0)
STRFY(P_MUL_FRONT(SP, 8))
"LDR R0, [SP, #4 * 29] \n\t"
STRFY(P_MUL_MID_OP_B_SHORT(SP, 12, R0, 4))
"LDR R0, [SP, #4 * 28] \n\t"
STRFY(P_MUL_MID_OP_A_SHORT(SP, 14, R0, 12))
STRFY(P_MUL_BACK2(SP, 16))  


//ROUND#3
"LDR R0, [SP, #4 * 28] \n\t"// OP_A
"LDR R1, [SP, #4 * 29] \n\t"// OP_B
STRFY(P_LOAD(R0, P_OP_A0, P_OP_A1, P_OP_A2, P_OP_A3, 4))
STRFY(P_LOAD2(R1, P_OP_B0, P_OP_B1, 0))
STRFY(P_MUL_FRONT(SP, 4))
"LDR R0, [SP, #4 * 29] \n\t"
STRFY(P_MUL_MID_OP_B(SP, 8, R0, 4))
STRFY(P_MUL_MID_OP_B_SHORT(SP, 12, R0, 8))
"LDR R0, [SP, #4 * 28] \n\t"
STRFY(P_MUL_MID_OP_A_SHORT(SP, 14, R0, 8))
STRFY(P_MUL_MID_OP_A2(SP, 16, R0, 10))
STRFY(P_MUL_BACK2(SP, 20)) 


//ROUND#4
"LDR R0, [SP, #4 * 28] \n\t"// OP_A
"LDR R1, [SP, #4 * 29] \n\t"// OP_B
STRFY(P_LOAD(R0, P_OP_A0, P_OP_A1, P_OP_A2, P_OP_A3, 0))
STRFY(P_LOAD(R1, P_OP_B0, P_OP_B1, P_OP_B2, P_OP_B3, 0))
STRFY(P_MUL_FRONT(SP, 0))
"LDR R0, [SP, #4 * 29] \n\t"
STRFY(P_MUL_MID_OP_B(SP, 4, R0, 4))
STRFY(P_MUL_MID_OP_B(SP, 8, R0, 8))
STRFY(P_MUL_MID_OP_B_SHORT(SP, 12, R0, 12))
"LDR R0, [SP, #4 * 28] \n\t"
STRFY(P_MUL_MID_OP_A_SHORT(SP, 14, R0, 4))
STRFY(P_MUL_MID_OP_A2(SP, 16, R0, 6))
STRFY(P_MUL_MID_OP_A2(SP, 20, R0, 10))
STRFY(P_MUL_BACK2(SP, 24)) 

//TEST
//"MOV R1, #0			   \n\t"//CARRY
//"ADDS R1, R1, R1	\n\t"
"LDR R0, [SP, #4 * 30] \n\t"//RESULT POINTER

STRFY(RED(30))

"ADD SP, #4*31 		   \n\t"
STRFY(P_MUL_EPILOG)
//////////////////////////////
  :
  : 
  : "cc", "memory"
);
}
	
void  __attribute__ ((noinline, naked)) fp_add434(u32* a, u32* b, u32* c) {
asm volatile(\
STRFY(P_MUL_PROLOG)
"SUB SP, #4*28 			\n\t"
STRFY(ADD_ASM)
"ADD SP, #4*31 		   \n\t"
STRFY(P_MUL_EPILOG)
//////////////////////////////
  :
  : 
  : "cc", "memory"
);
}

void  __attribute__ ((noinline, naked)) fp_sub434(u32* a, u32* b, u32* c) {
asm volatile(\
STRFY(P_MUL_PROLOG)
"SUB SP, #4*28 			\n\t"

STRFY(SUB_ASM)

"ADD SP, #4*31 		   \n\t"
STRFY(P_MUL_EPILOG)
//////////////////////////////
  :
  : 
  : "cc", "memory"
);
}

void   fp_add(u32* c, u32* a, u32* b) {
	fp_add434(a,b,c);
}

void   fp_sub(u32* c, u32* a, u32* b) {
	fp_sub434(a,b,c);
}

void   fp_mul(u32* c, u32* a, u32* b) {
	fp_mul434(a,b,c);
}

void   fp_sqr(u32* c, u32* a) {
	fp_mul434(a,a,c);
}

void fp_inv(u32* c, u32* z){
	u32 z3[28]={0,}, t0[28]={0,},  t2[28]={0,};
	int i;
		
	//z3:=z^(2^1)*z;
	fp_sqr(z3,z);
	fp_mul(z3,z3,z);
		
	//t0:=z3^(2^2)*z3;
	fp_sqr(t0,z3);
	fp_sqr(t0,t0);
	fp_mul(t0,t0,z3);
		
	//t1:=t0^(2^1)*z;
	//fp_sqr(t1,t0);
	fp_sqr(z3,t0);
	//fp_mul(t1,t1,z);
	fp_mul(z3,z3,z);
		
	//t2:=t1^(2^4)*t0;
	//fp_sqr(t2,t1);
	fp_sqr(t2,z3);
	for(i=0; i<3;i++){
		fp_sqr(t2,t2);
	}
	fp_mul(t2,t2,t0);
		
	//t3:=t2^(2^9)*t2;
	//fp_sqr(t3,t2);
	fp_sqr(z3,t2);
	for(i=0; i<8;i++){
		//fp_sqr(t3,t3);
		fp_sqr(z3,z3);
	}
	//fp_mul(t3,t3,t2);
	fp_mul(z3,z3,t2);
		
	//t4:=((t3^(2^18))*t3)^2*z;
	//fp_sqr(t4,t3);
	//fp_sqr(t4,z3);
	fp_sqr(t0,z3);
	for(i=0; i<17;i++){
		//fp_sqr(t4,t4);
		fp_sqr(t0,t0);
	}
	//fp_mul(t4,t4,t3);
	//fp_mul(t4,t4,z3);
	fp_mul(t0,t0,z3);
	//fp_sqr(t4,t4);
	fp_sqr(t0,t0);
	//fp_mul(t4,t4,z);
	fp_mul(t0,t0,z);
		
	//t5:=((t4^(2^37))*t4)^(2^37)*t4;
	//fp_sqr(t5,t4);
	//fp_sqr(z3,t4);
	fp_sqr(z3,t0);
	for(i=0; i<36;i++){
		//fp_sqr(t5,t5);
		fp_sqr(z3,z3);
	}
	//fp_mul(t5,t5,t4);
	//fp_mul(z3,z3,t4);
	fp_mul(z3,z3,t0);
	for(i=0; i<37;i++){
		//fp_sqr(t5,t5);
		fp_sqr(z3,z3);
	}
	//fp_mul(t5,t5,t4);
	//fp_mul(z3,z3,t4);
	fp_mul(z3,z3,t0);
		
	//t6:=t5^(2^111)*t5;
	//fp_sqr(t6,t5);
	//fp_sqr(t6,z3);
	fp_sqr(t0,z3);
	for(i=0; i<110;i++){
		//fp_sqr(t6,t6);
		fp_sqr(t0,t0);
	}
	//fp_mul(t6,t6,t5);
	//fp_mul(t6,t6,z3);
	fp_mul(t0,t0,z3);
		
	//t7:=((t6^(2^1)*z)^(2^223)*t6)^(2^2)*z;
	//fp_sqr(c,t6);
	fp_sqr(c,t0);
	fp_mul(c,c,z);
	for(i=0; i<223;i++){
		fp_sqr(c,c);
	}
	//fp_mul(c,c,t6);
	fp_mul(c,c,t0);
		
	fp_sqr(c,c);
	fp_sqr(c,c);
		
	fp_mul(c,c,z);
	
}



void PointAdd448(POINT448 *R, POINT448 *P, POINT448 *Q){
	u32* x1 = P->x;
	u32* y1 = P->y;
	u32* z1 = P->z;
	u32* e1 = P->e;
	u32* h1 = P->h;
	
	u32* u2 = Q->x;
	u32* v2 = Q->y;
	u32* w2 = Q->z;
	
	u32* x3 = R->x;
	u32* y3 = R->y;
	u32* z3 = R->z;
	u32* e3 = R->e;
	u32* h3 = R->h;
	
	fp_mul(t1,e1,h1);	//t1 := e1*h1;
	fp_sub(e3,y1,x1);	//e3 := y1-x1;
	fp_add(h3,y1,x1);	//h3 := y1+x1;
	fp_mul(x3,e3,v2);	//x3 := e3*v2;  // A = (y1-x1)*(y2-x2)
	fp_mul(y3,h3,u2);	//y3 := h3*u2;  // B = (y1+x1)*(y2+x2)
	fp_sub(e3,y3,x3);	//e3 := y3-x3;  // E = B-A
	fp_add(h3,y3,x3);	//h3 := y3+x3;  // H = B+A
	fp_mul(x3,t1,w2);	//x3 := t1*w2;  // C = t1*2*d*t2
	fp_sub(t1,z1,x3);	//t1 := z1-x3;  // F = z1-C
	fp_add(x3,z1,x3);	//x3 := z1+x3;  // G = z1+C
	
	fp_mul(z3,t1,x3);	//z3 := t1*x3;  // Z3 = F*G
	fp_mul(y3,x3,h3);	//y3 := x3*h3;  // Y3 = G*H
	fp_mul(x3,e3,t1);	//x3 := e3*t1;  // X3 = E*F
}

void PointDbl448(POINT448 *R, POINT448 *P){
	u32* x1 = P->x;
	u32* y1 = P->y;
	u32* z1 = P->z;
	
	u32* x3 = R->x;
	u32* y3 = R->y;
	u32* z3 = R->z;
	u32* e3 = R->e;
	u32* h3 = R->h;
	
	fp_sqr(e3,x1);	//e3 := x1*x1;  // A = x1*x1;
	fp_sqr(h3,y1);	//h3 := y1*y1;  // B = y1*y1;
	fp_sub(t1,e3,h3);//t1 := e3-h3;  // G = A-B (Hisil: G = -A+B)
	fp_add(h3,e3,h3);//h3 := e3+h3;  // H = A+B (Hisil: H = -A-B)
	fp_add(x3,x1,y1);//x3 := x1+y1;
	fp_sqr(e3,x3);	//e3 := x3*x3;
	fp_sub(e3,h3,e3);//e3 := h3-e3;  // E = H-(x1+y1)*(x1+y1) = -2*x1*x1 (Hisil: E = 2*x1*y1)
	fp_sqr(y3,z1);	//y3 := z1*z1;
	fp_add(y3,y3,y3);//y3 := 2*y3;   // C := 2*z1*z1;
	fp_add(y3,t1,y3);//y3 := t1+y3;  // F := G+C = A-B+C (Hisil: G = -A+B-C)
	
	fp_mul(x3,e3,y3);//x3 := e3*y3;  // X3 := E*F;
	fp_mul(z3,y3,t1);//z3 := y3*t1;  // Z3 := F*G;
	fp_mul(y3,t1,h3);//y3 := t1*h3;  // Y3 := G*H;
	
}

void ProToAff448(POINT448* R, POINT448* P){
	u32* xp = P->x;
	u32* yp = P->y;
	u32* zp = P->z;
	
	u32* xa = R->x;
	u32* ya = R->y;
	
	fp_inv(t1,zp);      //t1 := 1/zp;
	
	fp_mul(xa,xp,t1);//xa := xp*t1;
	fp_mul(ya,yp,t1);//ya := yp*t1;
	
}

void PointXZMulSecure448(POINT448* R, u32* k, POINT448* P){
	
	
	
	memcpy(T[0].x, one_div_2, sizeof(d));
	memcpy(T[0].y, one_div_2, sizeof(d));
	memset(T[0].z, 0, sizeof(d));
	
	fp_add(T[1].x, P->x, P->y);
	//fp_div(T[1].x,T[1].x);
	fp_mul(T[1].x, T[1].x, one_div_2);
	
	
	fp_sub(T[1].y, P->y, P->x);
	
	//
	
	
	//fp_div(T[1].y,T[1].y);
	fp_mul(T[1].y, T[1].y, one_div_2);
	
	fp_mul(T[1].z, P->x, P->y);
	fp_mul(T[1].z, T[1].z, d);
	
	memcpy(Q.x, P->x, sizeof(d));
	memcpy(Q.y, P->y, sizeof(d));
	memset(Q.z, 0, sizeof(d));
	Q.z[0] = 1;
	memcpy(Q.e, P->x, sizeof(d));
	memcpy(Q.h, P->y, sizeof(d));
	int i;
	for ( i=446; i>-1; i--) {
		PointDbl448(&Q, &Q);
		int ki = get_bit(k,i);
		PointAdd448(&Q, &Q, &T[ki]);
		
	}
	ProToAff448(R,&Q);
}
//



u32 px[]={0x7928111C,	0x3E12D50D,	0xAA886CAA,	0xD656860,	0x89DA0816,	0x2947D400,	0x4FB5FD4E,	0x4443C7EC,	0x558F011B,	0x80B86BF9,	0x3E29610C,	0x8D851BDC,	0x6402A2FD,	0x12D76E28};
u32 py[]={0x49C60372,	0x2EA3A45F,	0xA0E7FAA5,	0xEC86BB2F,	0x3AB8C350,	0xAF6558D4,	0xD1B4185B,	0x17BADD8A,	0x849015E9,	0xCE5679B,	0xA7CE82C,	0x5C3362E7,	0x7E800248,	0x9A8CFE9A};
u32 pz[] = {0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
u32 k_in[]={0x1CD47029,0xD45AACD9,0xEDA1E330,0xA948BAA3,0xBE133D95,0xFB03165C,0x9D819C39,0x6F65205B,0x9BD7E32D,0x17188C5F,0x156E9636,0x8C724968,0xB25800A3,0xABFF2714};

POINT448 P_IN, R_OUT;

int main(void)
{

	memcpy(P_IN.x,px,sizeof(px));
	memcpy(P_IN.y,py,sizeof(py));
	memcpy(P_IN.z,pz,sizeof(pz));

	PointXZMulSecure448(&R_OUT,k_in,&P_IN);
 
  return 0;
}

