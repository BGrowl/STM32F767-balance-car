#include "motor.h"

#define Max_PWM 21599
#define Min_PWM -21599

void motor_control_L(int speed)
{
	if(speed > Max_PWM)
		{speed = Max_PWM;}
	if(speed < Min_PWM)
		{speed = Min_PWM;}
		
	if(speed > 0)//正转 小车向前
	{
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_RESET);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, speed);

	}
	else         //反转 小车向后
	{
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_SET);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, -speed);
	}

		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}

void motor_control_R(int speed)
{
	if(speed > Max_PWM)
		{speed = Max_PWM;}
	if(speed < Min_PWM)
		{speed = Min_PWM;}
	
	if(speed > 0)//正转 小车向前
	{
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_5, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_6, GPIO_PIN_SET);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, speed);
	}
	else         //反转 小车向后
	{
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_5, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_6, GPIO_PIN_RESET);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, -speed);
	}

		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}



		














































