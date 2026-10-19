# 控制原理与调参

[返回首页](../README.md)

## 控制一次如何发生

MPU6050 DMP 的 `DEFAULT_MPU_HZ` 配置为 100。PH4 外部中断只设置数据标志，主循环再读取姿态、角速度和编码器，计算控制量。这是约 10 ms 的目标采样周期；阻塞串口操作和主循环延迟会影响实际执行周期。

```text
PH4 中断置位
    → 读取 pitch / roll / yaw、gx / gy / gz
    → 读取 TIM5 左编码器和 TIM4 右编码器，再清零计数
    → Balance_PD(pitch, gy)
    → Velocity_PI(left_encoder, right_encoder)
    → Turn_PD(gz)
    → 左右 PWM 合成、限幅、输出
```

核心逻辑位于 `Core/Src/main.c`，算法位于 `Drivers/BSP/PID/pid.c`。

## 直立 PD

代码定义 `Angle_bias = Mid_Angle - pitch`、`Gyro_bias = -gy`，然后计算：

```text
Vertical_out = -Balance_Kp × Angle_bias - Balance_Kd × Gyro_bias
```

角度项负责回到机械平衡位置，角速度项提供阻尼。`gy` 是驱动读出的原始角速度数据，不能直接当作已经换算为 °/s 的物理量；参数大小与传感器量程、安装方向及代码中的符号定义有关。

## 速度 PI

将左右编码器和取负，经过一阶低通，再积分：

```text
Encoder_Least = -(left_encoder + right_encoder)
Encoder_bias = 0.84 × Encoder_bias + 0.16 × Encoder_Least
Encoder_Integral += Encoder_bias + Movement
Encoder_Integral 限制在 [-8000, 8000]
Velocity_out = Velocity_Kp × Encoder_bias + Velocity_Ki × Encoder_Integral
```

前进与后退改变 `Movement_Target`，`Movement` 每次控制计算以 1.5 为步长趋近目标。这些速度数值属于当前代码的计数/控制尺度，未根据轮径和减速比换算为 m/s。

## 转向与 PWM 合成

转向目标由遥控状态选择。直行和后退时使用 `gz` 角速度反馈；其他状态将转向微分系数置零。`Turn_PD` 的函数名不表示所有运动状态都启用了完整 PD。

```text
Turn_out = Turn_Target × Turn_Kp + gz × Kd
PWM_out = Vertical_out + Velocity_out
Motor_L_PWM = PWM_out + Turn_out
Motor_R_PWM = PWM_out - Turn_out
```

电机驱动将每路输出限制到 `[-21599, 21599]`。TIM3 的预分频为 0，周期为 21599；按当前 108 MHz 定时器时钟可推得 PWM 频率约 5 kHz。

## 当前默认参数

以 `Drivers/BSP/PID/pid.c` 的变量定义为准，`main.c` 中保留的旧参数注释不是当前默认值。

| 参数 | 当前值 | 作用 |
| --- | ---: | --- |
| `Balance_Kp` | 800 | 直立比例 |
| `Balance_Kd` | 4.8 | 直立阻尼 |
| `Velocity_Kp` | 300 | 速度比例 |
| `Velocity_Ki` | 1.5 | 速度积分 |
| `Turn_Kp` | 200 | 转向比例 |
| `Turn_Kd` | 5 | 直行/后退角速度反馈 |
| `Mid_Angle` | -7 | 初始机械平衡中值（角度） |
| `Car_Target_Velocity` | 100 | 前后运动目标幅度 |
| `Car_Turn_Amplitude_speed` | 20 | 普通左右转目标幅度 |

`$5,...#` 指令会把 `Mid_Angle` 改成 `0.2` 并停止运动目标；这属于当前视觉任务的特殊配置，不能直接认定适合其他车体。

## 建议调参顺序

1. **先确认反馈方向。** 架空检查电机、编码器与姿态轴；错误符号会形成正反馈。
2. **标定机械中值。** 根据实际重心和 MPU 安装姿态调整 `Mid_Angle`，不要通过增大速度环去掩盖中值偏差。
3. **先直立环。** 临时隔离速度与转向输出，小幅调整比例和阻尼；车体能恢复直立后再进行下一步。
4. **再速度环。** 恢复速度输出，逐步抑制持续漂移。注意积分限幅及当前代码未在倾倒时清积分。
5. **最后运动与转向。** 从较小速度和转向幅度开始，再接入视觉控制。

每次只改一类参数，记录电池电压、重心、地面、参数和现象。串口查询与密集打印会占用主循环，不应在平衡调试时持续输出大量文本。
