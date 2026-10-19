#include "app_bluetooth.h"
#include "bsp_bluetooth.h"



#define 	run_car     '1'//按键前 Before pressing the button
#define 	back_car    '2'//按键后 After pressing the button
#define 	left_car    '3'//按键左 Left button
#define 	right_car   '4'//按键右 Right button
#define 	stop_car    '0'//按键停 Press the button to stop
#define 	ball_mid_angle '5'//进入找球阶段 Set middle angle for ball mode

//上报数据 Reporting data
int g_autoup = 0;
char manydisplay[80] ={0};
char updata[80] ={0};
char lspeed[10],rspeed[10],daccel[10],dgyro[10],csb[10],vi[10];

uint8_t newLineReceived = 0;
int num = 0;
uint8_t startBit = 0;
int int9num =0;
uint8_t inputString[80] = {0};
uint8_t ProtocolString[80] = {0};

enCarState g_newcarstate = enSTOP; //  1前2后3左4右0停止 1 forward 2 backward 3 left 4 right 0 stop


//PID部分  PID section
extern float Balance_Kp,Balance_Kd,Velocity_Kp,Velocity_Ki,Turn_Kp,Turn_Kd,Mid_Angle; //引入立直环和速度环,转向环 //Introducing vertical rings, speed rings, and steering rings
char piddisplay[50] ="$AP";
char charkp[10],charkd[10],charksp[10],charksi[10] ,charktp[10],charktd[10];
float PID_Original[6] = {0};

int StringFind(const char *pSrc, const char *pDst)  
{  
    int i, j;  
    for (i=0; pSrc[i]!='\0'; i++)  
    {  
        if(pSrc[i]!=pDst[0])  
            continue;         
        j = 0;  
        while(pDst[j]!='\0' && pSrc[i+j]!='\0')  
        {  
            j++;  
            if(pDst[j]!=pSrc[i+j])  
            break;  
        }  
        if(pDst[j]=='\0')  
            return i;  
    }  
    return -1;  
}  


//函数功能：保留6个PID的初始值  Function Function: Retain the initial values of 6 PIDs
void Init_PID(void)
{
	PID_Original [0] = Balance_Kp;
	PID_Original [1] = Balance_Kd;
	PID_Original [2] = Velocity_Kp;
	PID_Original [3] = Velocity_Ki;
	PID_Original [4] = Turn_Kp;
	PID_Original [5] = Turn_Kd;
}

//函数功能:恢复开机的PID值  Function function: Restore the PID value when turned on
void ResetPID(void)
{	
	if(Balance_Kp != PID_Original[0])
	{
		Balance_Kp = PID_Original[0];
	}
	if(Balance_Kd != PID_Original[1])
	{
		Balance_Kd = PID_Original[1];
	}
	if(Velocity_Kp != PID_Original[2])
	{
		Velocity_Kp = PID_Original[2];
	}
	if(Velocity_Ki != PID_Original[3])
	{
		Velocity_Ki = PID_Original[3];
	}
	if(Turn_Kp != PID_Original[4])
	{
		Turn_Kp = PID_Original[4];
	}
	if(Turn_Kd != PID_Original[5])
	{
		Turn_Kd = PID_Original[5];
	}
}	


void deal_bluetooth(uint8_t rxbuf)
{
		uint8_t uartvalue = rxbuf;
	
	 if(uartvalue == '$')
	    {
	      startBit = 1;
		    num = 0;
	    }
	    if(startBit == 1)
	    {
	       	inputString[num] = uartvalue;     
	    }  
	    if (startBit == 1 && uartvalue == '#') 
	    {
	    				
			newLineReceived = 1; 
			startBit = 0;
			int9num = num;	
		
	    }
		num++;
		if(num >= 80)
		{
			num = 0;
			startBit = 0;
			newLineReceived	= 0;
		}	 

}


void ProtocolCpyData(void)
{
	memcpy(ProtocolString, inputString, num+1);
	memset(inputString, 0x00, sizeof(inputString));
}

//不停的上报数据会影响平衡   //Continuously reporting data can affect balance
void Protocol(void)
{	
	switch (ProtocolString[1])
	{
		case run_car:	g_newcarstate = enRUN; if(APP_BLE == 1) {printf("run_car\r\n");}break;
		case back_car:  g_newcarstate = enBACK; if(APP_BLE == 1) {printf("back_car\r\n");}break;
		case left_car:  g_newcarstate = enLEFT; if(APP_BLE == 1) {printf("left_car\r\n");}break;
		case right_car: g_newcarstate = enRIGHT;if(APP_BLE == 1) {printf("right_car\r\n");}break;
		case stop_car:  g_newcarstate = enSTOP; if(APP_BLE == 1) {printf("stop_car\r\n");}break;
		case ball_mid_angle: Mid_Angle = 0.2f; g_newcarstate = enSTOP; if(APP_BLE == 1) {printf("ball_mid_angle\r\n");}break;
		default: g_newcarstate = enSTOP; break;
	}
	if (ProtocolString[3] == '1') //左旋 Left-handed
	{
		g_newcarstate = enTLEFT;	
        if(APP_BLE == 1) {printf("Left-handed\r\n");}
	}
	
	if (ProtocolString[3] == '2') //右旋 Right
	{
		g_newcarstate = enTRIGHT;
        if(APP_BLE == 1) {printf("Right\r\n");}
	}

//	/*防止数据丢包 Preventing Data Loss */
	if(strlen((const char *)ProtocolString)<21)
	{
		newLineReceived = 0;  
		memset(ProtocolString, 0x00, sizeof(ProtocolString));  
		UART5_Send_Char("$ReceivePackError#"); //返回协议数据包  Return protocol data packet
		return;
	}

	
	//查询PID   Query PID
	if(ProtocolString[5]=='1')
	{
		ProtocolGetPID(); //app bug 发送2次  App bug sent 2 times
		HAL_Delay(5);
		ProtocolGetPID();
	}
	else if(ProtocolString[5]=='2')  //恢复默认PID  Restore the default PID
	{
		ResetPID();
		ProtocolGetPID(); //在发送一次pid  Sending a pid
		UART5_Send_Char("$OK#");//返回协议数据包  Return protocol data packet
	}

	//自动上报  Automatic reporting
	if(ProtocolString[7]=='1')
	{
			g_autoup = 1; 
			UART5_Send_Char("$OK#"); //返回协议数据包  	Return protocol data packet
	}
	else if(ProtocolString[7]=='2')
	{		
			g_autoup = 0;		
			UART5_Send_Char("$OK#"); //返回协议数据包  Return protocol data packet	 	
	}

	//更新PID的参数  Update PID parameters
	if (ProtocolString[9] == '1') //角度环更新   Angular Ring Update   $0,0,0,0,1,1,AP23.54,AD85.45,VP10.78,VI0.26,TP0.12,TD0.00#
	{
		//$0,0,0,0,1,1,AP23.54,AD85.45,VP10.78,VI0.26,TP0.12,TD0.00#

		int pos,z; 
		char apad[25] = {0},apvalue[8] = {0},advalue[8] = {0};
			
		pos = StringFind((const char *)ProtocolString, (const char *)"AP");
		if(pos == -1) return;
		
		memcpy(apad,ProtocolString+pos,int9num-pos);

		//AP23.54,AD85.45,VP10.78,VI0.26,TP0.12,TD0.00#
		z = StringFind(apad, ",");
		if(z == -1) return;
		memcpy(apvalue, apad+2, z-2);
		
		Balance_Kp = atof(apvalue)*100; //*100是放到100倍  *100 means magnify 100 times
		
		memset(apad, 0x00, sizeof(apad));
		memcpy(apad, ProtocolString + pos + z + 1, int9num - (pos + z)); //存储AD后面的数据 Store data after AD
		z = StringFind(apad, ",");
		if(z == -1) return;
		memcpy(advalue,apad+2, z-2);
		
		Balance_Kd=atof(advalue); //默认的值就是放大100倍  The default value is to magnify 100 times.
        
        if(APP_BLE == 1) {printf("B_Kp %.1f,B_Kd %.1f\r\n",Balance_Kp,Balance_Kd);}//Balance_Kp,Balance_Kd,Velocity_Kp,Velocity_Ki,Turn_Kp,Turn_Kd;

        UART5_Send_Char("$OK#"); //返回协议数据包   Return protocol packet
    }
		
  	if(ProtocolString[11] == '1')  //此解析要微改  This analysis needs to be slightly modified
	{
		int pos,z; 
		char vpvi[25] = {0},vpvalue[8] = {0},vivalue[8] = {0};
			
		pos = StringFind((const char *)ProtocolString, (const char *)"VP");
		if(pos == -1) return;
		
		memcpy(vpvi, ProtocolString+pos, int9num-pos);
		//y=strchr(apad,'AP');
		//AP23.54,AD85.45,VP10.78,VI0.26,TP0.12,TD0.00#
		z = StringFind(vpvi, ",");
		if(z == -1) return;
		memcpy(vpvalue, vpvi+2, z-2);
		
		Velocity_Kp = atof(vpvalue) *100;//*100是放到100倍  *100 means magnify 100 times
		
		
		memset(vpvi, 0x00, sizeof(vpvi));
		memcpy(vpvi, ProtocolString + pos + z + 1, int9num - (pos + z)); //存储AD后面的数据  Store data after AD
		z = StringFind(vpvi, ","); //添加了转向环后 #变,    After adding the turning ring # changes,
		if(z == -1) return;
		memcpy(vivalue,vpvi+2, z-2);
		
		Velocity_Ki=atof(vivalue);//默认的值就是放大100倍  The default value is to magnify 100 times.

        if(APP_BLE == 1) {printf("V_Kp %.1f,V_Ki %.1f\r\n",Velocity_Kp,Velocity_Ki);}//Balance_Kp,Balance_Kd,Velocity_Kp,Velocity_Ki,Turn_Kp,Turn_Kd;


		UART5_Send_Char("$OK#"); //返回协议数据包  	Return protocol data packet	
			
			
	}
	
	//解析转向环的数据 Analyze the data of the steering ring
	 if(ProtocolString[13] == '1')
	{
		int pos,z; 
		char tptd[25] = {0},tpvalue[8] = {0},tdvalue[8] = {0};
			
		pos = StringFind((const char *)ProtocolString, (const char *)"TP");
		if(pos == -1) return;
		
		memcpy(tptd, ProtocolString+pos, int9num-pos);
		
		z = StringFind(tptd, ",");
		if(z == -1) return;
		memcpy(tpvalue, tptd+2, z-2);
		
		Turn_Kp = atof(tpvalue) *100;//*100是放到100倍   *100 means magnify 100 times
		
        if(APP_BLE == 1) {printf("T_Kp %.1f,T_Kd %.1f\r\n",Turn_Kp,Turn_Kd);}//Balance_Kp,Balance_Kd,Velocity_Kp,Velocity_Ki,Turn_Kp,Turn_Kd;
		
        
		memset(tptd, 0x00, sizeof(tptd));
		memcpy(tptd, ProtocolString + pos + z + 1, int9num - (pos + z)); //存储AD后面的数据  Store data after AD
		z = StringFind(tptd, "#");
		if(z == -1) return;
		memcpy(tdvalue,tptd+2, z-2);
		
		Turn_Kd = atof(tdvalue);//因为app bug,默认的值就是放大100倍  Because of the app bug, the default value is to magnify 100 times

		UART5_Send_Char("$OK#"); //返回协议数据包  Return protocol packet		
			
			
	}
	
	
	
	
	newLineReceived = 0;  
	memset(ProtocolString, 0x00, sizeof(ProtocolString));  


}


float s_Acc = 0, s_Gyro = 0;
void CalcUpData(void)
{
	float ls, rs,sLence;
	
	if(g_autoup == 1)
	{
//		ls = Velocity_Left;//左电机速度  left speed
//		rs = Velocity_Right;//右电机速度  right speed
//		s_Acc = Acceleration_Z/100; //加速度  acceleration //为了能在app显示完全，缩小100倍  In order to fully display it in the app, it is reduced by 100 times
//		s_Gyro = Gyro_Balance; //陀螺仪  gyroscope
//		sLence = g_distance/10.0; //超声波  ultrasonic
		
	
//		printf("ls:%.2f\t,rs:%.2f,acc:%.2f,gryo:%.2f,dis:%.2f \r\n",ls,rs,s_Acc,s_Gyro,sLence);
		memset(manydisplay, 0x00, 80);
		memcpy(manydisplay, "$LV", 4);
	
		memset(lspeed, 0x00, sizeof(lspeed));
		memset(rspeed, 0x00, sizeof(rspeed));
		memset(daccel, 0x00, sizeof(daccel));
		memset(dgyro, 0x00, sizeof(dgyro));
		memset(csb, 0x00, sizeof(csb));
		memset(vi, 0x00, sizeof(vi));
	
		//左边速度  left speed
		if((ls <= 1000) && (ls >= -1000))
			sprintf(lspeed,"%3.2f",ls);
		else
		{
			return;
		}
			
		//右边速度 right speed
		if((rs <= 1000) && (rs >= -1000))
			sprintf(rspeed,"%3.2f",rs);
		else
		{	
			return;
		}
		
		//角速度  acc
		if((s_Acc > -2000) && (s_Acc < 2000))
			sprintf(daccel,"%3.2f",s_Acc);
		else
		{
			return;
		}
		
		//陀螺仪 gryo
		if((s_Gyro > -10000) && (s_Gyro < 10000))
			sprintf(dgyro,"%3.2f",s_Gyro);
		else
		{
			return;
		}
	
		//超时波距离  ultrasonic
		if((sLence >= 0) && (sLence < 10000))
			sprintf(csb,"%3.2f",(float)sLence);
		else
		{
			return;
		}
		
		//电量  quantity of electricity
//		if((battery >= 0) && (battery < 20))
//			sprintf(vi,"%3.2f",battery);
//		else
//		{
//			return;
//		}
	
		strcat(manydisplay,lspeed);
		strcat(manydisplay,",RV");
		strcat(manydisplay,rspeed);
		strcat(manydisplay,",AC");
		strcat(manydisplay,daccel);
		strcat(manydisplay,",GY");
		strcat(manydisplay,dgyro);
		strcat(manydisplay,",CSB");
		strcat(manydisplay,csb);
		strcat(manydisplay,",VT");
		strcat(manydisplay,vi);
		strcat(manydisplay,"#");
		memset(updata, 0x00, 80);
		memcpy(updata, manydisplay, 80);
	}
	
}


//自动上报  Automatic reporting
int g_uptimes = 1; //自动上报 2秒报一次 Automatically report every 2 seconds
void SendAutoUp(void)
{
	g_uptimes --;
	if ((g_autoup == 1) && (g_uptimes == 0))
	{
		CalcUpData();
		UART5_Send_Char(updata); //返回协议数据包	Return protocol data packet
	}
	if(g_uptimes == 0)
		 g_uptimes = 1;

}


//查讯PID Query PID
void ProtocolGetPID(void)
{
	memset(piddisplay, 0x00, sizeof(piddisplay));
	memcpy(piddisplay, "$AP", 4);

	if(Balance_Kp >= 0 ) //&& Balance_Kp <= 28800  
	{
		sprintf(charkp,"%3.2f",Balance_Kp/100);
	}
	else
	{	
		UART5_Send_Char("$GetPIDError#"); //返回协议数据包  Return protocol data packet
		return;
	}

	
	if(Balance_Kd >= 0 ) //&& Balance_Kd <= 100
	{
		sprintf(charkd,"%3.2f",Balance_Kd);//此值原生传过去 This value is passed natively
	}
	else
	{	
		UART5_Send_Char("$GetPIDError#"); //返回协议数据包  Return protocol data packet
		return;
	}
	
	if(Velocity_Kp >= 0 ) //&& Velocity_Kp <= 20000
	{
		sprintf(charksp,"%3.2f",Velocity_Kp/100);
	}
	else
	{	
		UART5_Send_Char("$GetPIDError#"); //返回协议数据包  Return protocol data packet
		return;
	}

	if(Velocity_Ki >= 0 ) //&& Velocity_Ki <= 100
	{
		sprintf(charksi,"%3.2f",Velocity_Ki); //此值原生传过去  This value is passed natively
	}
	else
	{	
		UART5_Send_Char("$GetPIDError#"); //返回协议数据包 Return protocol data packet
		return;
	}
	
	 
	//转向环 TP  Steering ring TP
	if(Turn_Kp >= 0 ) 
	{
		sprintf(charktp,"%3.2f",Turn_Kp/100); 
	}
	else
	{	
		UART5_Send_Char("$GetPIDError#"); //返回协议数据包  Return protocol data packet
		return;
	}
	
	//转向环 TD  Steering ring TD
	if(Turn_Kd >= 0 ) 
	{
		sprintf(charktd,"%3.2f",Turn_Kd); 
	}
	else
	{	
		UART5_Send_Char("$GetPIDError#"); //返回协议数据包  Return protocol data packet
		return;
	}
	
	
	strcat(piddisplay,charkp);
	strcat(piddisplay,",AD");
	strcat(piddisplay,charkd);
	strcat(piddisplay,",VP");
	strcat(piddisplay,charksp);
	strcat(piddisplay,",VI");
	strcat(piddisplay,charksi);
	
	//添加转向环  Add steering ring
	strcat(piddisplay,",TP");
	strcat(piddisplay,charktp);
	strcat(piddisplay,",TD");
	strcat(piddisplay,charktd);
	strcat(piddisplay,"#");
	
	UART5_Send_Char(piddisplay); //返回协议数据包  Return protocol data packet

}



