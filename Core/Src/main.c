/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
	
	/*
	TIM3输出PWM控制速度 
	TIM4 TIM5用于读取增量编码器 
	IIC1接口读取MPU6050
	接线注释：
	MPU6050  VCC->>-5V GND->>-GND  SCL->>-PB8 SDA->>-PB7 INT->>-PH4 (IIC数字接口 数据处理完成中断)
	电机1  M+ ->>- AO2  M- ->>-AO1  VCC ->>- 3V3 GND ->>- GND B ->> E1B A ->>- E1A
	电机2  M+ ->>- BO1  M- ->>-BO2  VCC ->>- 3V3 GND ->>- GND B ->> E2B A ->>- E2A
	电机驱动板 STBY ->>- 3V3  PWMA ->>- PA6  PWMB ->>- PA7  (电机调节速度)
						E1A ->>- PD13 E1B ->>- PD12      (电机1增量编码器)
						E2A ->>- PH10 E2B ->>- PH11 		 (电机2增量编码器)
						5V ->>- 5V  GND ->>-GND 需要共地
						AIN1 ->>- PI5   AIN2 ->>- PI6    (控制电机1正反转)
						BIN1 ->>- PG13  BIN2 ->>- PG14   (控制电机2正反转) 
	CH340等USB转串口模块 可连接PA9 PA10 读取串口信息，串口波特率115200
	*/
	
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "app_bluetooth.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mpu6050.h"
#include "motor.h"
#include "stdio.h"
#include "pid.h"
#include "usbd_cdc_if.h" // 包含 CDC 发送函数的声明
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t Rx1_Temp;

float pitch, roll, yaw;
short gx, gy, gz;
int32_t Motor_L_counter,Motor_R_counter;
int Vertical_out,Velocity_out,Turn_out;             //直立环&速度环&转向环的输出变量
int PWM_out,Motor_L_PWM,Motor_R_PWM;
_Bool MPU_Data_read_flag = 0;

//注意：设置为0时候使用串口1 引脚PA9(TX) PA10(RX) 需要使用USB转TTL模块（例如CH340）连接串口1引脚 才可使用printf 波特率9600 可连接蓝牙模块 
//设置为1时候 使用板载usb mirco接口 虚拟USB接口，可进行printf函数打印进行数据监测
//请务必注意，当设置为1时，由于USB CDC特性导致，当且仅当使用xcom（或其他上位机软件）连接打开虚拟串口后程序才正常运行。
#define USB_CDC_PRINTF 0

//使用蓝牙模块连接需要配置#define USB_CDC_PRINTF 0
//使用亚博智能APP控制，请务必设置为		APP_BLE = 0
uint8_t  APP_BLE = 0;         //设置为0后请使用亚博智能app蓝牙连接，设置为1或其他可以使用蓝牙串口微信小程序（蓝牙串口）进行蓝牙连接 可以作为无线串口printf数据
//连接蓝牙模块，请使用5V供电，可使用电机驱动板的V out 5V供电，但需驱动板与核心板共地（GND ->>- GND）RX引脚连接PA9 TX引脚连接PA10  

//小车运动指令
/*
//stop
$0,0,0,0,0,0,0,0,0,0#
//qian
$1,0,0,0,0,0,0,0,0,0#
//hou
$2,0,0,0,0,0,0,0,0,0#
//zuo
$3,0,0,0,0,0,0,0,0,0#
//you
$4,0,0,0,0,0,0,0,0,0#

设置PID参数指令  $0,0,0,0,1,1,1,AP8.0,AD5.8,VP4.0,VI1.5,TP3.0,TD50#
float Balance_Kp =800;
float Balance_Kd =5.8;
//速度环PI控制参数
float Velocity_Kp=400;
float Velocity_Ki=1.5;  //kp/200
//转向环PD控制参数
float Turn_Kp=300; 
float Turn_Kd=50;

//查询PID指令
$0,0,1,0,0,0,0,0,0,0#

*/

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#if USB_CDC_PRINTF
// 记得在 Target 选项中勾选 "Use MicroLIB" 以减小代码体积
int fputc(int ch, FILE *f)
{
    uint8_t result = USBD_OK;
    uint8_t temp[1] = {ch};

    do {
        // 调用 USB HS 发送函数
        result = CDC_Transmit_HS(temp, 1);
    } while (result == USBD_BUSY);

    return ch;
}

#else

#ifdef __GNUC__									/// 重定向 printf 到 USART1
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart1 , (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
#endif



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_4) { MPU_Data_read_flag = 1;}//请不要在中断中调取硬件IIC读取 中断快进快出 请勿使用printf等阻塞型函数
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
	
	printf("AT+NAME YahBoom_BL\r\n");//原始蓝牙模块 名称 ，可以连接亚博智能APP进行控制，其他蓝牙名称APP不识别。
//  printf("AT+NAME BLE_CAR\r\n");//自定义蓝牙模块名称  
	
  Init_PID();
	HAL_UART_Receive_IT(&huart1, &Rx1_Temp, 1);

  printf("MPU 6050 init start\n");  
  if(MPU_Init() == 0) {printf("MPU 6050 init ok\n"); }
    else              {printf("MPU 6050 init fail\n");}
  printf("DMP library init start\n"); 
  if(mpu_dmp_init() == 0) {printf("DMP init ok\n"); }
    else                  {printf("DMP fail\n");}
    
   HAL_TIM_Base_Start(&htim3);//电机 PMW 定时器
	 HAL_TIM_Base_Start(&htim4);//编码器定时器初始化
	 HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
	 HAL_TIM_Base_Start(&htim5);//编码器定时器初始化
	 HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);
	 printf("STM32_READY\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
		//while函数内禁止使用HAL_Delay
  // printf("Hello STM32 Serial Is Working!\r\n");
    
		while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_4);

	if (newLineReceived == 1) 
        {
            // 第一步：把中断里的数据拷贝出来
            // 因为 inputString 随时会被新的中断修改，所以要先拷贝到 ProtocolString
            ProtocolCpyData(); 
            // 第二步：解析协议并执行动作
            Protocol(); 
        }
	if(MPU_Data_read_flag == 1)	//100hz MPU6050中断触发
    {
		MPU_Data_read_flag = 0;               //数据标志位
		mpu_dmp_get_data(&pitch, &roll, &yaw);//读取DMP数据 转换为姿态数据
        MPU_Get_Gyroscope(&gx, &gy, &gz);     //读取加速度
    
		Motor_L_counter = -(int32_t)__HAL_TIM_GET_COUNTER(&htim5);//读取增量式编码器数值
		Motor_R_counter = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
		__HAL_TIM_SET_COUNTER(&htim5,0);//清空编码器计数值
		__HAL_TIM_SET_COUNTER(&htim4,0);	

		Vertical_out =  Balance_PD(pitch,gy);//
		Velocity_out = Velocity_PI(Motor_L_counter,Motor_R_counter);
		//Velocity_out = 0;
		Turn_out = Turn_PD(gz);	

		PWM_out=Vertical_out + Velocity_out;                                      
			
		Motor_L_PWM=PWM_out+Turn_out;                                             //左电机PWM
		Motor_R_PWM=PWM_out-Turn_out;                                             //右电机PWM    

		motor_control_L(Motor_L_PWM);//Motor_L_PWM
		motor_control_R(Motor_R_PWM);//Motor_R_PWM
			
//	  printf("%.1f,%d,%d,%d,%d\n", pitch,gy,Vertical_out,Motor_L_counter,Motor_R_counter);
//printf("L:%d, R:%d\r\n", Motor_L_counter, Motor_R_counter);	
//    printf("pitch:%.1f roll:%.1f yaw:%.1f x:%d y:%d z:%d\r\n", pitch, roll, yaw, gx, gy, gz);
  // printf("%.1f,%.1f,%.1f,%d,%d,%d\r\n", pitch, roll, yaw, gx, gy, gz);
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 12;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
