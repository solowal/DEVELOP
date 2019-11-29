
//84MHz

#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef unsigned long u32;
typedef unsigned char u8;

int i;
unsigned long time1;
unsigned long time2;

extern u32 __attribute__ ((noinline, naked)) enc_intra_asm(u8* input, u8* round_key, u8* random_num) ;
extern u32 __attribute__ ((noinline, naked)) dec_intra_asm(u8* input, u8* round_num, u8* random_num) ;
extern void __attribute__ ((noinline, naked)) key_asm(u8* input, u8* round_key,  u8* delta) ;

extern void __attribute__ ((noinline, naked)) key_dup(u8* input, u8* outputx2);


// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);       //serial communication initializing
}

u8 input[16]={0XD7,   0X6D,   0X0D,   0X18,   0X32,   0X7E,   0XC5,   0X62};
u8 inputx2[16]={0XD7,   0X6D,   0X0D,   0X18,   0X32,   0X7E,   0XC5,   0X62, 0XD7,   0X6D,   0X0D,   0X18,   0X32,   0X7E,   0XC5,   0X62};
u8 pbUserKey[16] = {0x88, 0xE3, 0x4F, 0x8F, 0x08, 0x17, 0x79, 0xF1, 0xE9, 0xF3, 0x94, 0x37, 0x0A, 0xD4, 0x05, 0x89 };     // User secret key
//u8 round_key[136]={0X0A,  0XD4,   0X05,   0X89,   0X88,   0XE3,   0X4F,   0X8F,   0XE2,   0X50,   0X85,   0XAA,   0X15,   0X1D,   0X7C,   0X32,   0X49,   0X23,   0XAC,   0X83,   0X70,   0X07,   0X5E,   0XB5,   0X47,   0XB3,   0XF8,   0X99,   0XF4,   0X7A,   0X50,   0X95,   0XD7,   0X50,   0X66,   0X0D,   0X73,   0X68,   0X43,   0X3C,   0XD4,   0X1E,   0X9E,   0XEE,   0X54,   0XD1,   0X29,   0X67,   0X2D,   0XDD,   0X13,   0X48,   0XFE,   0XAC,   0X84,   0X51,   0X55,   0XD8,   0X20,   0X9F,   0X2E,   0X74,   0XE1,   0X31,   0XE8,   0X0F,   0XCE,   0X4B,   0X24,   0XEC,   0XA3,   0X80,   0X43,   0X34,   0X87,   0X38,   0XEB,   0X54,   0XC7,   0X0B,   0X88,   0X53,   0X44,   0XA8,   0XF8,   0XFA,   0XD7,   0X98,   0XFF,   0X40,   0X73,   0XE7,   0X68,   0X03,   0X20,   0X6D,   0X86,   0X31,   0X27,   0X6E,   0XBD,   0X03,   0X40,   0XBA,   0X62,   0XD8,   0X2C,   0X29,   0X82,   0XF5,   0X8A,   0XE4,   0XD4,   0X57,   0X1A,   0XDC,   0X49,   0XAB,   0XFA,   0X3B,   0X47,   0X81,   0XA8,   0X14,   0X5D,   0X9C,   0X42,   0XF0,   0X67,   0XCE,   0X94,   0X38,   0X2B,   0X70,   0XBE,   0X43};
u8 round_key[136]={0,};
u8 round_keyx2[272]={0,};

u8 DELTA[128] = 
{
  0x5A, 0x6D, 0x36, 0x1B, 0x0D, 0x06, 0x03, 0x41,
  0x60, 0x30, 0x18, 0x4C, 0x66, 0x33, 0x59, 0x2C,
  0x56, 0x2B, 0x15, 0x4A, 0x65, 0x72, 0x39, 0x1C,
  0x4E, 0x67, 0x73, 0x79, 0x3C, 0x5E, 0x6F, 0x37,
  0x5B, 0x2D, 0x16, 0x0B, 0x05, 0x42, 0x21, 0x50,
  0x28, 0x54, 0x2A, 0x55, 0x6A, 0x75, 0x7A, 0x7D,
  0x3E, 0x5F, 0x2F, 0x17, 0x4B, 0x25, 0x52, 0x29,
  0x14, 0x0A, 0x45, 0x62, 0x31, 0x58, 0x6C, 0x76,
  0x3B, 0x1D, 0x0E, 0x47, 0x63, 0x71, 0x78, 0x7C,
  0x7E, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x43, 0x61,
  0x70, 0x38, 0x5C, 0x6E, 0x77, 0x7B, 0x3D, 0x1E,
  0x4F, 0x27, 0x53, 0x69, 0x34, 0x1A, 0x4D, 0x26,
  0x13, 0x49, 0x24, 0x12, 0x09, 0x04, 0x02, 0x01,
  0x40, 0x20, 0x10, 0x08, 0x44, 0x22, 0x11, 0x48,
  0x64, 0x32, 0x19, 0x0C, 0x46, 0x23, 0x51, 0x68,
  0x74, 0x3A, 0x5D, 0x2E, 0x57, 0x6B, 0x35, 0x5A
};

u8 random_num[4] = {0x5A, 0x6D, 0x36, 0x1B}; 

u32 fault_test = 1;
// the loop routine runs over and over again forever:
void loop() {
/*
  time1 = millis();
  for(i=0;i<100000;i++){
    //key_asm(pbUserKey, round_key, DELTA);
    enc_asm(inputx2,round_keyx2);
  }
  time2 = millis();
  Serial.println("HIGHT encryption");
  Serial.println((time2-time1));
/**/
  delay(2000);
  key_asm(pbUserKey, round_key, DELTA);
  key_dup(round_key,round_keyx2);
  /*
  Serial.println("Result of HIGHT encryption");
  for(int i=0;i<272;i++){
    Serial.println(round_keyx2[i], HEX);
  }
  */
 /* */
  Serial.println("Result of HIGHT encryption");
  for(int i=0;i<16;i++){
    Serial.println(inputx2[i], HEX);
  }
  fault_test= enc_intra_asm(inputx2,round_keyx2,random_num);
  Serial.println("Result of HIGHT encryption");
  for(int i=0;i<16;i++){
    Serial.println(inputx2[i], HEX);
  }

  Serial.println(fault_test, HEX);

  
  fault_test = dec_intra_asm(inputx2,round_keyx2,random_num);

 
  Serial.println("Result of HIGHT encryption");
  for(int i=0;i<16;i++){
    Serial.println(inputx2[i], HEX);
  }

  Serial.println(fault_test, HEX);
  
  delay(500000);
/**/  

}
