# 许可范围与第三方来源

本项目由应用代码、STM32Cube 生成代码和多种第三方组件构成。根目录 [LICENSE](LICENSE) 的 MIT 授权仅覆盖本次新增的说明文档，不覆盖整个固件，也不改变既有文件的版权。

## 已有声明

| 组件 | 本地声明 | 处理方式 |
| --- | --- | --- |
| ARM CMSIS | `Drivers/CMSIS/LICENSE.txt`，Apache-2.0 | 保留许可和文件版权头；个别历史文件以自身声明为准 |
| STM32F7 设备支持 | `Drivers/CMSIS/Device/ST/STM32F7xx/LICENSE.txt` | 保留随附声明 |
| STM32 HAL | `Drivers/STM32F7xx_HAL_Driver/LICENSE.txt` | 随附文本指向软件包许可；脱离软件包且无其他条款时指定 BSD-3-Clause |
| ST USB Device Library | `Middlewares/ST/STM32_USB_Device_Library/LICENSE.txt` | 保留随附声明与源码头 |
| STM32Cube 生成文件 | `Core/`、`USB_DEVICE/` 等的文件头 | 保留 ST 版权与许可提示，不改授 MIT |
| InvenSense eMPL/DMP | `Drivers/BSP/MPU6050/eMPL/` 文件头 | 保留 InvenSense 版权；所引用 `License.txt` 未随原项目提供 |
| 既有 BSP、K230 脚本、图片和图示脚本 | 未发现统一的授权文本 | 保留原文件；不推定所有代码均为仓库所有者原创 |

## 待补充信息

原目录未提供项目自身原创部分与外部参考代码的完整边界、版本和来源。部分注释涉及 YahBoom 蓝牙协议及历史移植代码，不能据此确定其全部权利归属。

尤其是 `inv_mpu.c`、`inv_mpu.h`、`inv_mpu_dmp_motion_driver.*` 明确引用了未附带的 `License.txt`。本仓库没有为这些文件补造授权，也没有将其他版本的 TDK 协议套用到这些历史文件。需要从原始供方补齐对应版本的许可，才能确定可再分发和使用范围。

TDK 当前的 [开发者入口](https://www.invensense.tdk.com/en-us/developers) 与 [软件许可页面](https://www.invensense.tdk.com/en-us/software-license-agreement) 可作为向供方查询的入口，不能替代当前目录对应的历史许可。

在这些信息补齐前，本仓库应视为**项目源码公开与复现资料**，不能宣称所有组件均已获得统一开源授权。二次分发和商业使用请逐项核对相应权利与许可。
