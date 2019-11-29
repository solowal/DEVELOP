/*
 * Copyright (C) 2011-2018 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include "Enclave.h"
#include "Enclave_t.h"  /* print_string */

#include "sgx_tseal.h"
#include "sgx_trts.h"

typedef unsigned int u32;
typedef unsigned char u8;

#define LEA_NUM_RNDS		24
#define LEA_KEY_BYTE_LEN	16

#define LEA_BLK_BYTE_LEN	16
#define LEA_RNDKEY_WORD_LEN	6

u32 ROR(u32 W, u32 i){
	return (((W)>>(i)) | ((W)<<(32-(i))));
}
u32 ROL(u32 W, u32 i){
	return (((W)<<(i)) | ((W)>>(32-(i))));
}

/*******************************************************************************/

#define u32_in(x)            (*(u32*)(x))
#define u32_out(x, v)        {*((u32*)(x)) = (v);}

/*******************************************************************************/

void LEA_Keyschedule(u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN],
					 const u8 pbKey[LEA_KEY_BYTE_LEN])
{
	u32 delta[4] = {0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec};
	u32 T[4] = {0x0,};

	T[0] = u32_in(pbKey);
	T[1] = u32_in(pbKey + 4);
	T[2] = u32_in(pbKey + 8);
	T[3] = u32_in(pbKey + 12);

	for(int i=0; i<LEA_NUM_RNDS; i++)
	{
		T[0] = ROL(T[0] + ROL(delta[i&3], i), 1);
		T[1] = ROL(T[1] + ROL(delta[i&3], i+1), 3);
		T[2] = ROL(T[2] + ROL(delta[i&3], i+2), 6);
		T[3] = ROL(T[3] + ROL(delta[i&3], i+3), 11);

		pdRndKeys[i][0] = T[0];
		pdRndKeys[i][1] = T[1];
		pdRndKeys[i][2] = T[2];
		pdRndKeys[i][3] = T[1];
		pdRndKeys[i][4] = T[3];
		pdRndKeys[i][5] = T[1];
	}
}

/*******************************************************************************/

void LEA_EncryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN],
					const u8 pbSrc[LEA_BLK_BYTE_LEN],
					const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN])
{
	u32 X0,X1,X2,X3;
	u32 temp;

	X0 = u32_in(pbSrc);
	X1 = u32_in(pbSrc + 4);
	X2 = u32_in(pbSrc + 8);
	X3 = u32_in(pbSrc + 12);

	for(int i=0; i<LEA_NUM_RNDS; i++)
	{
		X3 = ROR((X2 ^ pdRndKeys[i][4]) + (X3 ^ pdRndKeys[i][5]), 3);
		X2 = ROR((X1 ^ pdRndKeys[i][2]) + (X2 ^ pdRndKeys[i][3]), 5);
		X1 = ROL((X0 ^ pdRndKeys[i][0]) + (X1 ^ pdRndKeys[i][1]), 9);
		temp = X0;
		X0 = X1; X1 = X2; X2 = X3; X3 = temp;
	}

	u32_out(pbDst, X0);
	u32_out(pbDst + 4, X1);
	u32_out(pbDst + 8, X2);
	u32_out(pbDst + 12, X3);
}

/*******************************************************************************/

void LEA_DecryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN],
					const u8 pbSrc[LEA_BLK_BYTE_LEN],
					const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN])
{
	u32 X0,X1,X2,X3;
	u32 temp;

	X0 = u32_in(pbSrc);
	X1 = u32_in(pbSrc + 4);
	X2 = u32_in(pbSrc + 8);
	X3 = u32_in(pbSrc + 12);


	for(int i=0; i<LEA_NUM_RNDS; i++)
	{
		temp = X3;
		X3 = X2;
		X2 = X1;
		X1 = X0;
		X0 = temp;

		X1 = (ROR(X1,9) - (X0 ^ pdRndKeys[LEA_NUM_RNDS-1-i][0])) ^ pdRndKeys[LEA_NUM_RNDS-1-i][1];
		X2 = (ROL(X2,5) - (X1 ^ pdRndKeys[LEA_NUM_RNDS-1-i][2])) ^ pdRndKeys[LEA_NUM_RNDS-1-i][3];
		X3 = (ROL(X3,3) - (X2 ^ pdRndKeys[LEA_NUM_RNDS-1-i][4])) ^ pdRndKeys[LEA_NUM_RNDS-1-i][5];

	}

	u32_out(pbDst, X0);
	u32_out(pbDst + 4, X1);
	u32_out(pbDst + 8, X2);
	u32_out(pbDst + 12, X3);
}

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}

void LEA_test(unsigned char* key, unsigned char* text){
	//printf("TEST\t %d", a);
	u8 src2[LEA_BLK_BYTE_LEN] = { 0x0, };
	
	u32 rndkeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN] = {0x0,};
	LEA_Keyschedule(rndkeys, key);
	LEA_EncryptBlk(src2, text, rndkeys);
	int i;
	printf("Ciphertext\t");
	for(i=0;i<16;i++){
		printf("0x%X, ", src2[i]);
	}printf("\n");
	
}

void printf_helloworld()
{
	
	uint8_t p_hash[32]={0,};
	int i=0;
	
	const uint8_t p_key[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	const uint8_t p_src[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	uint32_t src_len = 16;
	uint8_t p_ctr[]={0,1,2,3};
	const uint32_t ctr_inc_bits=1;
	uint8_t p_dst[16]={0,};
	
	
	printf("AES128-CTR result\n");
	printf("####################\n");
	sgx_aes_ctr_encrypt( (sgx_aes_ctr_128bit_key_t*) p_key, p_src, src_len, p_ctr, ctr_inc_bits, p_dst);
	for(i=0;i<16;i++){
		printf("0x%X, ", p_dst[i]);
	}printf("\n");
	
	printf("####################\n");
	
	
	/*
	sgx_sha256_msg(p_src,3,(sgx_sha256_hash_t *) p_hash);
	printf("SHA256 result\n");
	printf("####################\n");
	
	for(i=0;i<32;i++){
		printf("0x%X, ", p_hash[i]);
	}printf("\n");
	
	printf("####################\n");
	*/
	
}

	//uint8_t rand_m[33]={0,};
	
	//printf("%c%c\n",p_hash[0],p_hash[1]);
	//printf("%x %x\n",rand_m[0],rand_m[1]);
	//sgx_read_rand(rand_m,10);
    //printf("Hello World\n");

