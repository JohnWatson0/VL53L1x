#include "iic.h"


#define TRUE 1
#define FALSE 0

static uint8_t IIC_Start(void);				
static void IIC_Stop(void);	  		
static void IIC_Send_Byte(uint8_t txd);	
static uint8_t IIC_Read_Byte(void);
static uint8_t IIC_Wait_Ack(void); 	
static void IIC_Ack(void);				
static void IIC_NAck(void);		


void iic_init(void) {
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin	= VL53_SCL_Pin;
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(VL53_SCL_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin	= VL53_SDA_Pin;
	GPIO_Init(VL53_SDA_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_SetBits(VL53_SCL_GPIO_Port, VL53_SCL_Pin);
	GPIO_SetBits(VL53_SDA_GPIO_Port, VL53_SDA_Pin);
}

static void IIC_delay(void)
{
    uint16_t i=3;//���72Mhz��Ƶ�������ʵ���ʱ
   while(i)
   {
     i--;
   }
}


/**************************ʵ�ֺ���********************************************
*����ԭ��:		void IIC_Start(void)
*��������:		����IIC��ʼ�ź�
*******************************************************************************/
static uint8_t IIC_Start(void)
{
//	SDA_OUT();
	SDA_H;
	SCL_H;
//	SDA_IN();
	IIC_delay();
	if(!SDA_read)
		return FALSE;	
	SDA_L;
	IIC_delay();
	if(SDA_read)
		return FALSE;	
	SDA_L;
	IIC_delay();
	return TRUE;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void IIC_Stop(void)
*��������:	    //����IICֹͣ�ź�
*******************************************************************************/
static void IIC_Stop(void)
{
	SCL_L;
	IIC_delay();
	SDA_L;
	IIC_delay();
	SCL_H;
	IIC_delay();
	SDA_H;
	IIC_delay();
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		uint8_t IIC_Wait_Ack(void)
*��������:	    �ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
*******************************************************************************/
static uint8_t IIC_Wait_Ack(void) 	
{
	SCL_L;
	IIC_delay();
	SDA_H;
	IIC_delay();
	SCL_H;
	IIC_delay();
	if(SDA_read)
	{
    SCL_L;
	  IIC_delay();
      return FALSE;
	}
	SCL_L;
	IIC_delay();
	return TRUE;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void IIC_Ack(void)
*��������:	    ����ACKӦ��
*******************************************************************************/
static void IIC_Ack(void)
{
	SCL_L;
	IIC_delay();
	SDA_L;
	IIC_delay();
	SCL_H;
	IIC_delay();
	SCL_L;
	IIC_delay();
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void IIC_NAck(void)
*��������:	    ����NACKӦ��
*******************************************************************************/
static void IIC_NAck(void)
{
	SCL_L;
	IIC_delay();
	SDA_H;
	IIC_delay();
	SCL_H;
	IIC_delay();
	SCL_L;
	IIC_delay();
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void IIC_Send_Byte(uint8_t txd)
*��������:	    IIC����һ���ֽ�
*******************************************************************************/
static void IIC_Send_Byte(uint8_t SendByte) 
{
    uint8_t i=8;
    while(i--)
    {
			SCL_L;
			IIC_delay();
			if(SendByte&0x80)
				SDA_H;
			else
				SDA_L;
			SendByte<<=1;
			IIC_delay();
			SCL_H;
			IIC_delay();
    }
    SCL_L;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		uint8_t IIC_Read_Byte(unsigned char ack)
*��������:	    //��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
*******************************************************************************/
static unsigned char IIC_Read_Byte(void)  
{
    uint8_t i=8;
    uint8_t ReceiveByte=0;

    SDA_H;
    while(i--)
    {
			ReceiveByte<<=1;
			SCL_L;
			IIC_delay();
			SCL_H;
			IIC_delay();
			if(SDA_read)
			{
				ReceiveByte|=0x01;
			}
    }
    SCL_L;
    return ReceiveByte;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		unsigned char IIC_ReadOneByte(unsigned char I2C_Addr,unsigned char addr)
*��������:	    ��ȡָ���豸 ָ���Ĵ�����һ��ֵ
����	I2C_Addr  Ŀ���豸��ַ
		addr	   �Ĵ�����ַ
����   ��������ֵ
*******************************************************************************/
uint8_t IIC_ReadOneByte(uint8_t SlaveAddress,uint16_t REG_Address,uint8_t* data)
{
	if(!IIC_Start())
			return FALSE;
    IIC_Send_Byte(SlaveAddress); 
    if(!IIC_Wait_Ack())
		{
			IIC_Stop();
			return FALSE;
		}
    IIC_Send_Byte((uint8_t) REG_Address>>8);   
    IIC_Wait_Ack();
    IIC_Send_Byte((uint8_t) REG_Address & 0x00ff);   
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte(SlaveAddress+1);
    IIC_Wait_Ack();

	*data= IIC_Read_Byte();
    IIC_NAck();
    IIC_Stop();
    return TRUE;
}


/**************************ʵ�ֺ���********************************************
*����ԭ��:		uint8_t IICreadBytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data)
*��������:	    ��ȡָ���豸 ָ���Ĵ����� length��ֵ
����	dev  Ŀ���豸��ַ
		reg	  �Ĵ�����ַ
		length Ҫ�����ֽ���
		*data  ���������ݽ�Ҫ��ŵ�ָ��
����   ���������ֽ�����
*******************************************************************************/
uint8_t IICreadBytes(uint8_t SlaveAddress,uint16_t REG_Address,uint8_t len,uint8_t *data)
{
		uint8_t i = 0;
		if(!IIC_Start())
			return FALSE;
    IIC_Send_Byte(SlaveAddress); 
    if(!IIC_Wait_Ack())
		{
			IIC_Stop();
			return FALSE;
		}
    IIC_Send_Byte((uint8_t) REG_Address>>8);   
    IIC_Wait_Ack();
    IIC_Send_Byte(REG_Address&0x00ff);   
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte(SlaveAddress+1);
    IIC_Wait_Ack();

		for(i = 0;i<len;i++)
		{
			if(i != (len -1))
			{
				data[i]= IIC_Read_Byte();
				IIC_Ack();
			}
			else
			{
				data[i]= IIC_Read_Byte();
				IIC_NAck();
			}
		}
		IIC_Stop();
		return len;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		uint8_t IICwriteBytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data)
*��������:	    ������ֽ�д��ָ���豸 ָ���Ĵ���
����	dev  Ŀ���豸��ַ
		reg	  �Ĵ�����ַ
		length Ҫд���ֽ���
		*data  ��Ҫд�����ݵ��׵�ַ
����   ����д�볤��
*******************************************************************************/
uint8_t IICwriteBytes(uint8_t dev, uint16_t reg, uint16_t length, uint8_t* data)
{

 	uint8_t count = 0;
	IIC_Start();
	IIC_Send_Byte(dev);	   
	IIC_Wait_Ack();
	IIC_Send_Byte(reg>>8);   
    IIC_Wait_Ack();
	IIC_Send_Byte(reg & 0x00ff);   
    IIC_Wait_Ack();
	for(count=0;count<length;count++)
	{
		IIC_Send_Byte(data[count]);
		IIC_Wait_Ack();
	 }
	IIC_Stop();

    return length; //status == 0;
}


/**************************ʵ�ֺ���********************************************
*����ԭ��:		unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char data)
*��������:	    д��ָ���豸 ָ���Ĵ���һ���ֽ�
����	dev  Ŀ���豸��ַ
		reg	   �Ĵ�����ַ
		data  ��Ҫд����ֽ�
����   1
*******************************************************************************/
uint8_t IICwriteByte(uint8_t dev, uint16_t reg, uint8_t data)
{
    return IICwriteBytes(dev, reg, 1, &data);
}


/**************************ʵ�ֺ���********************************************
*����ԭ��:		uint8_t IICwriteBit(uint8_t dev, uint8_t reg, uint8_t bitNum, uint8_t data)
*��������:	    �� �޸� д ָ���豸 ָ���Ĵ���һ���ֽ� �е�1��λ
����	dev  Ŀ���豸��ַ
		reg	   �Ĵ�����ַ
		bitNum  Ҫ�޸�Ŀ���ֽڵ�bitNumλ
		data  Ϊ0 ʱ��Ŀ��λ������0 ���򽫱���λ
����   �ɹ� Ϊ1
 		ʧ��Ϊ0
*******************************************************************************/
uint8_t IICwriteBit(uint8_t dev, uint16_t reg, uint8_t bitNum, uint8_t data)
{
    uint8_t b;
    IIC_ReadOneByte(dev, reg, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return IICwriteByte(dev, reg, b);
}


