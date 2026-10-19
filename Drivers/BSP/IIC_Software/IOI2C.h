#ifndef __IOI2C_H
#define __IOI2C_H

#include "AllHeader.h"

//IO口操作宏定义 IO port operation macro definition
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))  

//IO口地址映射  IO port address mapping
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出    Output
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入    Input

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出   Output
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入   Input

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出   Output
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入   Input


//IO方向设置   IO direction setting
//#define SDA_IN()  {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=8<<12;}
//#define SDA_OUT() {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=3<<12;}

////IO操作函数	   IO operation function
//#define IIC_SCL    PBout(10) //SCL
//#define IIC_SDA    PBout(11) //SDA	 
//#define READ_SDA   PBin(11)  //输入SDA   Input SDA

//因为硬件发送交换 Because the hardware sends the exchange
#define SDA_IN()  {GPIOB->CRH&=0XFFFFF0FF;GPIOB->CRH|=8<<8;}
#define SDA_OUT() {GPIOB->CRH&=0XFFFFF0FF;GPIOB->CRH|=3<<8;}

//IO操作函数	 IO operation function
#define IIC_SCL    PBout(11) //SCL
#define IIC_SDA    PBout(10) //SDA	 
#define READ_SDA   PBin(10)  //输入SDA   Input SDA

//IIC所有操作函数  IIC all operation functions
void IIC_MPU6050_Init(void);           //初始化IIC的IO口		  //Initialize IIC IO port		 
int IIC_Start(void);					 //发送IIC开始信号  //Send IIC start signal
void IIC_Stop(void);	  			 //发送IIC停止信号  //Send IIC stop signal
void IIC_Send_Byte(u8 txd);		 //IIC发送一个字节  //IIC sends a byte
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节  //IIC reads a byte
int IIC_Wait_Ack(void); 			 //IIC等待ACK信号  //IIC waits for ACK signal
void IIC_Ack(void);						 //IIC发送ACK信号  //IIC sends ACK signal
void IIC_NAck(void);					 //IIC不发送ACK信号  //IIC does not send ACK signal

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	 
unsigned char I2C_Readkey(unsigned char I2C_Addr);

unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr);
unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char data);
u8 IICwriteBytes(u8 dev, u8 reg, u8 length, u8* data);
u8 IICwriteBits(u8 dev,u8 reg,u8 bitStart,u8 length,u8 data);
u8 IICwriteBit(u8 dev,u8 reg,u8 bitNum,u8 data);
u8 IICreadBytes(u8 dev, u8 reg, u8 length, u8 *data);

int i2cWrite(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data);
int i2cRead(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);

#endif

//------------------End of File----------------------------
