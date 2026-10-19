# 编译、烧录与首次运行

[返回首页](../README.md)

## 1. 准备开发环境

- Windows 与 Keil MDK/uVision。
- ARM Compiler 5。工程记录的版本为 `V5.06 update 3 (build 300)`；本次实际检查结果见 [验证记录](validation.md)。工程未完成 ARM Compiler 6 或 GCC 迁移验证。
- 设备包 `Keil.STM32F7xx_DFP.2.7.0`。工程还引用 CMSIS CORE 组件，缺包时请按 Keil 的缺失组件提示安装。
- ST-LINK 驱动、USB 转 TTL 驱动和串口终端。
- STM32CubeMX 为可选配置工具；编译现有工程无需先重新生成代码。

```sh
git clone https://github.com/BGrowl/STM32F767-balance-car.git
cd STM32F767-balance-car
```

## 2. 打开并编译

1. 打开 `MDK-ARM/STM32F767-balance-car.uvprojx`。
2. 选择目标 `STM32F767-balance-car`，确认器件为 `STM32F767IGTx`。
3. 在 Target 选项中选择已安装的 ARM Compiler 5，保留 MicroLIB 配置。
4. 检查 HSE 晶振：源码按 25 MHz HSE、216 MHz 系统时钟配置。若开发板不同，必须相应修改时钟设置。
5. 执行 **Rebuild all target files**，检查完整的错误和警告。

输出位于 `MDK-ARM/STM32F767-balance-car/`，包括 `.axf` 和 `.hex`。这些是本地构建产物，不随源码提交。不要用旧 HEX 判断当前源码编译是否成功。

可选命令行方式（按本机位置替换 UV4 路径）：

```powershell
& 'D:\MDK5\UV4\UV4.exe' -r 'MDK-ARM\STM32F767-balance-car.uvprojx' -o 'build.log'
```

Keil 的返回码需结合构建日志判断；出现警告时也可能返回非零值。

## 3. 烧录

连接 ST-LINK 的 SWDIO、SWCLK、GND 和开发板要求的电压参考。在 Keil 的 Debug/Utilities 选项中选择 ST-LINK，确认 Flash 算法匹配开发板，再执行 Download。

第一次烧录应架空车轮或断开电机功率电源，避免初始化完成后车轮突然转动。

## 4. 检查串口启动日志

USART1 使用 PA9（TX）和 PA10（RX），参数为 **9600 baud、8 数据位、无校验、1 停止位、无硬件流控**。旧 `main.c` 接线注释中的 115200 与当前初始化不同，以 `Core/Src/usart.c` 为准。

预期成功日志包含：

```text
MPU 6050 init start
MPU 6050 init ok
DMP library init start
DMP init ok
STM32_READY
```

`STM32_READY` 会在初始化调用后打印，不能单独证明 MPU6050 和 DMP 成功；还需检查前面的两项 `init ok`。程序启动时会发送 `AT+NAME YahBoom_BL`，属于现有蓝牙模块配置逻辑。

`main.c` 默认 `USB_CDC_PRINTF=0`，打印走 USART1；`APP_BLE=0`。若改为 USB CDC 打印，现有实现可能等待主机打开虚拟串口，并在发送忙时阻塞。首次复现建议保留默认配置。

## 5. 验证闭环方向

1. 架空车轮，轻微倾斜车身，观察姿态是否连续变化。
2. 手转两个车轮，检查编码器计数及符号。
3. 检查电机纠偏方向。方向错误时先核对接线、坐标轴和安装朝向。
4. 扶稳车身，按 [控制说明](control.md) 标定 `Mid_Angle`，再调直立环、速度环和转向环。
5. 最后接入遥控或 K230。停止命令用于停止运动目标，仍会保留平衡输出，不能当成断电急停。

## 常见问题

| 现象 | 检查方向 |
| --- | --- |
| 串口乱码 | 9600 8N1、共地、TX/RX 交叉、逻辑电平 |
| MPU 初始化失败 | PB7/PB8、供电、I2C 上拉、模块地址 |
| 有启动日志但无平衡输出 | PH4 中断线、DMP 初始化结果、STBY、电机电源 |
| 一上电就加速倒下 | 左右电机映射、编码器符号、MPU 安装方向、机械中值 |
| 连接 USB 后才开始工作 | 检查是否开启 `USB_CDC_PRINTF` |
| K230 无法导入 `media` | 需在兼容的 CanMV K230 固件运行，桌面 CPython 不提供该硬件 API |
| Keil 缺设备或编译器 | 安装对应 DFP 和 ARM Compiler 5，在工程中选择可用版本 |

重新用 CubeMX 生成代码前先提交或备份。自定义 BSP 和 `USER CODE` 区需要保留，不能假设重新生成后所有手工修改都能保留。
