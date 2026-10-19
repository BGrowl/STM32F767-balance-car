#include "bsp_key.h"

//uint16_t g_key1_long_press = 0;


//// 判断按键是否被按下，按下返回KEY_PRESS，松开返回KEY_RELEASE
//// Check if the key is pressed, return KEY_PRESS if pressed, return KEY_RELEASE if released
//static uint8_t Key1_is_Press(void)
//{
//	if (!GPIO_ReadInputDataBit(KEY1_GPIO_PORT, KEY1_GPIO_PIN))
//	{
//		return KEY_PRESS; // 如果按键被按下，则返回KEY_PRESS // If the key is pressed, returns KEY_PRESS
//	}
//	return KEY_RELEASE;   // 如果按键是松开状态，则返回KEY_RELEASE  // If the key is released, return KEY_RELEASE
//}


//// 读取按键K1的长按状态，累计达到长按时间返回1，未达到返回0. // Read the long press status of button K1. If the long press time is reached, return 1. If not, return 0.
//// timeout为设置时间长度，单位为秒  // timeout is the set time length, in seconds
//uint8_t Key1_Long_Press(uint16_t timeout)
//{
//	if (g_key1_long_press > 0)
//	{
//		if (g_key1_long_press < timeout * 100 + 2)
//		{
//			g_key1_long_press++;
//			if (g_key1_long_press == timeout * 100 + 2)
//			{
//				return 1;
//			}
//			return 0;
//		}
//	}
//	return 0;
//}




//// 读取按键K1的状态，按下返回1，松开返回0.
//// mode:设置模式，0：按下一直返回1；1：按下只返回一次1
////Read the status of button K1, press to return to 1, release to return to 0
////Mode: Set mode, 0: Press and hold to return 1; 1: Press once to return 1
//uint8_t Key1_State(uint8_t mode)
//{
//	static uint16_t key1_state = 0;

//	if (Key1_is_Press() == KEY_PRESS)
//	{
//		if (key1_state < (mode + 1) * 2)
//		{
//			key1_state++;
//		}
//	}
//	else
//	{
//		key1_state = 0;
//		g_key1_long_press = 0;
//	}
//	if (key1_state == 2)
//	{
//		g_key1_long_press = 1;
//		return KEY_PRESS;
//	}
//	return KEY_RELEASE;
//}




//// 初始化按键1 // Initialize button 1
//void Key1_GPIO_Init(void)
//{
////	/*定义一个GPIO_InitTypeDef类型的结构体  Define a GPIO_InitTypeDef type structure */
////	GPIO_InitTypeDef GPIO_InitStructure;
////	/*开启按键端口的时钟  Enable the clock of the button port */
////	RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK, ENABLE);
////	//选择按键的引脚  Select the pin of the button
////	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN;
////	//设置按键的引脚为输入  Set the button pin as input
////	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
////	//使用结构体初始化按键  Initialize buttons using structure
////	GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);
//}


//void KEYAll_GPIO_Init(void)
//{
//	Key1_GPIO_Init();
//	
//}


///*********************************************END OF FILE**********************/
