# 发布前验证记录

[返回首页](../README.md)

本记录区分静态检查、编译和实物运行。文档描述的是当前源码行为，不代表已经对当前车体完成全部实验。

## 本次检查（2026-10-19）

| 检查项 | 方式与结果 |
| --- | --- |
| Keil 工程源文件 | 检查 `.uvprojx` 列出的 47 个文件，路径全部存在 |
| 引脚与串口 | 对照 GPIO、TIM、I2C、USART 初始化与电机函数；串口为 9600 8N1 |
| 控制周期与参数 | 对照 `DEFAULT_MPU_HZ=100` 和 `pid.c` 实际定义，纠正文档中可能由历史注释造成的歧义 |
| K230 与图示脚本 | Python AST 语法检查；未在 K230 上执行 |
| 文档链接 | 检查 Markdown 本地文件链接与图片路径 |
| 完整固件重编译 | 使用本机 Keil 与 ARM Compiler 5，结果见下 |
| 整车与烧录 | 本次未连接、烧录或验证实际硬件 |

## 固件构建

本机使用 `D:\MDK5\UV4\UV4.exe` 对 `STM32F767-balance-car` 目标执行 Rebuild。首次构建为 **0 errors、1 warning**，警告来自 `bsp_bluetooth.c` 缺少文件末尾换行。发布整理补上换行，不改变运行逻辑，随后再次重编译并核对最终结果。

补齐换行后的最终重编译结果为 **0 Error(s), 0 Warning(s)**，Keil 返回码为 0。编译器为 **ARM Compiler 5.06 update 3 (build 300)**。

```text
Program Size: Code=41698 RO-data=4094 RW-data=556 ZI-data=15708
FromELF: creating hex file...
"STM32F767-balance-car\\STM32F767-balance-car.axf" - 0 Error(s), 0 Warning(s).
```

发布候选文件检查未发现所扫描的常见密钥格式、个人 Windows 用户目录路径和 ST-LINK 序列号配置。个人 `.uvoptx`/`.uvguix` 配置、缓存、未引用的预编译 DSP 库与其他课题资料不纳入提交；该检查不构成对所有潜在敏感信息的绝对保证。

随后从 Git 提交导出独立源码目录，再次执行完整性检查与 Keil Rebuild，结果同样为 **0 errors、0 warnings**，程序尺寸与上述结果一致。这一步确认编译不依赖原目录中被忽略的缓存或预编译 DSP 库。环境仍使用本机已安装的编译器和设备包。

## 仓库完整性检查

```sh
python tools/check_repository.py
```

该脚本使用 Python 标准库，检查必要文件、Keil 源文件路径、本地文档链接及两个 Python 脚本的语法。GitHub Actions 在 push/PR 时运行相同检查。它不编译 ARM 固件，也不验证算法效果或硬件 API。

## 已知限制和后续工作

- MPU 初始化失败后仍可能继续执行，主循环没有完善的传感器故障保护。
- 未实现完整的倾倒停机、速度积分复位、欠压保护和通信失联停车。
- 串口协议的异常帧校验、缓冲区边界与中断/主循环共享数据保护需要加强。
- PID 查询、USB CDC 和阻塞串口发送可能延迟控制计算；实际周期需在硬件测量。
- Keil 包含目录保留两个历史 OLED 路径，当前目录没有 OLED 文件，当前目标也未编译 OLED 功能。
- `IIC_Software` 为历史移植代码，未加入当前目标，不能直接当作已验证的 STM32F7 HAL 模块。
- 自动遥测代码未完成有效数据接入；不作为当前功能承诺。
- K230 固件版本和板卡型号未记录，颜色识别与串口负载需要实机确认。
- 缺少 InvenSense 历史许可原文和既有应用代码的完整来源记录，见 [第三方说明](../THIRD_PARTY_NOTICES.md)。
