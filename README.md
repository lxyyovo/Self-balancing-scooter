# STM32 两轮自平衡小车

基于 **STM32F103C8T6** 的自平衡小车：MPU6050 姿态解算 + 互补滤波 + 串级 PID，编码器闭环测速，OLED 实时显示三环参数，支持蓝牙 App 在线调参与波形回传。

> 工程基于 STM32CubeMX 生成 HAL 库框架，用户代码集中在 `App/`、`Hardware/`、`System/` 三个目录。

---

## 目录

- [功能特性](#功能特性)
- [硬件平台](#硬件平台)
- [引脚分配](#引脚分配)
- [定时器与中断](#定时器与中断)
- [目录结构](#目录结构)
- [控制原理](#控制原理)
- [通信协议](#通信协议)
- [编译与烧录](#编译与烧录)
- [调参与标定](#调参与标定)
- [已知限制](#已知限制)
- [许可说明](#许可说明)
- [致谢](#致谢)

---

## 功能特性

- **自平衡控制**：角度环 + 速度环 + 转向环，串级 PID 闭环
- **姿态估计**：MPU6050 六轴数据，互补滤波融合加速度计倾角与陀螺仪角速度
- **速度闭环**：霍尔编码器接口（TIM 编码器模式），实测左右轮转速
- **实时显示**：0.96" OLED 分三栏显示三环 Kp/Ki/Kd、目标值、实际值与输出
- **蓝牙调参与波形**：USART2 接蓝牙模块，配合江协科技「蓝牙串口」上位机在线改 PID 参数、绘制三环曲线，并用摇杆遥控行进与转向
- **安全保护**：俯仰角超过 ±50° 自动停机；输出限幅 + 积分限幅防积分饱和
- **按键启停**：独立按键切换运行/停止

---

## 硬件平台

| 项目 | 参数 |
|---|---|
| 主控 | STM32F103C8T6（Cortex-M3，LQFP48） |
| Flash / RAM | 64 KB / 20 KB |
| 系统时钟 | 72 MHz（HSE 8 MHz 经 PLL ×9） |
| 姿态传感器 | MPU6050（软件 I2C，从机地址 `0xD0`） |
| 显示 | 0.96" OLED SSD1306（软件 I2C） |
| 测速 | 霍尔/光电增量式编码器 ×2（44 计数/圈） |
| 电机驱动 | TB6612FNG 双路 H 桥（每路 2 个方向脚 + 1 个 PWM 脚） |
| 蓝牙 | 串口透传模块（HC-05/06 类），默认 9600 |
| 调试器 | ST-Link / J-Link（SWD） |

---

## 引脚分配

### 电机与编码器

| 功能 | 引脚 | 外设 / 模式 | 说明 |
|---|---|---|---|
| 左电机 PWM | PA0 | TIM2_CH1 | TB6612 `PWMA`，20 kHz |
| 右电机 PWM | PA1 | TIM2_CH2 | TB6612 `PWMB`，20 kHz |
| 左电机方向 | PB12 / PB13 | GPIO 推挽输出 | TB6612 `AIN1` / `AIN2`，正转 PB12 置高 |
| 右电机方向 | PB15 / PB14 | GPIO 推挽输出 | TB6612 `BIN1` / `BIN2`，正转 PB15 置高 |
| 左轮编码器 | PA6 / PA7 | TIM3_CH1 / CH2，编码器模式 TI12 | 输入滤波 15 |
| 右轮编码器 | PB6 / PB7 | TIM4_CH1 / CH2，编码器模式 TI12 | CH2 反相，输入滤波 15 |

### 通信

| 功能 | 引脚 | 外设 | 配置 |
|---|---|---|---|
| 调试串口 | PA9(TX) / PA10(RX) | USART1 | 9600 8N1，`printf` 重定向到此口 |
| 蓝牙串口 | PA2(TX) / PA3(RX) | USART2 | 9600 8N1，上位机协议通信 |
| 下载调试 | PA13(SWDIO) / PA14(SWCLK) | SWD | — |

### 传感器与人机交互

| 功能 | 引脚 | 模式 | 说明 |
|---|---|---|---|
| MPU6050 I2C | PB10(SCL) / PB11(SDA) | 软件 I2C，开漏输出 | `MyI2C` 驱动 |
| OLED I2C | PB8(SCL) / PB9(SDA) | 软件 I2C，开漏输出 | 独立总线，与 MPU6050 分开 |
| 按键 1 | PB1 | 上拉输入 | 启动 / 停止 |
| 按键 2 | PB0 | 上拉输入 | 预留 |
| 按键 3 | PA5 | 上拉输入 | 预留 |
| 按键 4 | PA4 | 上拉输入 | 预留 |
| LED | PC13 | 推挽输出 | 低电平点亮，运行时常亮 |

---

## 定时器与中断

| 定时器 | 配置 | 用途 |
|---|---|---|
| TIM1 | PSC=71, ARR=999 → **1 ms** 更新中断，抢占优先级 0 | 主时基：按键扫描 + 平衡控制 |
| TIM2 | PSC=35, ARR=99 → **20 kHz** PWM | 双电机调速 |
| TIM3 | 编码器模式 TI12 | 左轮测速 |
| TIM4 | 编码器模式 TI12（CH2 反相） | 右轮测速 |

中断优先级分组为 `NVIC_PRIORITYGROUP_2`：

| 中断 | 抢占 / 子优先级 |
|---|---|
| TIM1_UP | 0 / 0（最高） |
| USART1 | 1 / 0 |
| USART2 | 1 / 1 |

两个串口均采用**单字节中断接收**（`HAL_UART_Receive_IT`），在 `HAL_UART_RxCpltCallback` 中按 `Instance` 分发到各自驱动。

---

## 目录结构

```
├── App/                    应用层：算法与通信协议
│   ├── PID.c / PID.h           通用 PID 控制器（微分先行 + 限幅）
│   └── BlueSerial.c / .h       蓝牙上位机协议解析与收发
│
├── Core/                   CubeMX 生成的应用核心
│   ├── Inc/                    main.h, tim.h, usart.h, gpio.h ...
│   └── Src/
│       ├── main.c              初始化、平衡控制主逻辑、中断回调分发
│       ├── tim.c                TIM1~TIM4 配置
│       ├── usart.c              USART1 / USART2 配置
│       └── gpio.c               引脚电平与模式配置
│
├── Drivers/                ST HAL 库与 CMSIS
│
├── Hardware/               外设驱动，每模块一个子目录
│   ├── Encoder/                编码器测速
│   ├── Key/                    按键扫描
│   ├── LED/                    运行指示灯
│   ├── Motor/                  电机方向 + PWM 合成
│   ├── MPU6050/                六轴传感器（含 MPU6050_Reg.h 寄存器定义）
│   ├── MyI2C/                  软件模拟 I2C 底层
│   ├── OLED/                   OLED 驱动与字库
│   ├── PWM/                    PWM 占空比设置
│   └── Serial/                 调试串口（printf 重定向）
│
├── System/                 基础支撑
│   ├── Delay.c / .h            DWT 内核计数器实现的微秒级延时
│   └── Timer.c / .h            TIM1 时基启动
│
├── MDK-ARM/                Keil MDK 工程（pinghengche.uvprojx）
├── pinghengche.ioc         STM32CubeMX 工程
└── README.md
```

---

## 控制原理

主控制循环运行在 TIM1 的 1 ms 中断内，通过两个软件分频计数器分出 10 ms 与 50 ms 两个节拍：

```
TIM1 每 1ms 中断
├── Key_Tick()                        按键扫描（20 ms 一次）
├── Count0 分频 → 每 10ms：姿态与控制
│   ├── MPU6050_GetData()             读加速度 + 陀螺仪
│   ├── 互补滤波 → Angle               俯仰角估计
│   ├── 角度环 PID                     Angle → AvePWM
│   ├── 合成左右 PWM                   LeftPWM = AvePWM + DifPWM/2
│   │                                 RightPWM = AvePWM - DifPWM/2
│   └── Motor_SetPWM()                输出到电机
└── Count1 分频 → 每 50ms：速度与转向
    ├── Encoder_Get() → Left/RightSpeed
    ├── 速度环 PID                     AveSpeed → AnglePID.Target
    └── 转向环 PID                     DifSpeed → DifPWM
```

**串级结构**（速度环的输出作为角度环的目标）：

```
摇杆/蓝牙指令 ─► SpeedPID.Target ─┬─► SpeedPID.Out (±20) ─► AnglePID.Target
                                  │
                                  └─► AnglePID.Out (±100) ─► AvePWM ─┬─► 左电机
                                                                     └─► 右电机
摇杆转向指令 ─► TurnPID.Target
DifSpeed ────► TurnPID.Out (±50) ─► DifPWM ─► 左右差速
```

**互补滤波**（10 ms 周期，陀螺仪为主、加速度计为辅）：

```
AngleAcc  = -atan2(AX, AZ) / π × 180 + 8.1        // 加速度计解算俯仰角（含零位补偿）
AngleGyro = Angle + GY / 32768 × 2000 × 0.01      // 陀螺仪角速度积分（±2000°/s 量程）
Angle     = 0.01 × AngleAcc + 0.99 × AngleGyro    // α = 0.01
```

**PID 实现特点**（`App/PID.c`）：

- 微分先行：`Out -= Kd × (Actual - Actual1)`，避免目标突变引入微分冲击
- 积分限幅：`ErrorInt` 限制在 `[ErrorIntMin, ErrorIntMax]`，`Ki == 0` 时强制清零
- 输出限幅：`Out` 限制在 `[OutMin, OutMax]`
- 输出死区补偿：`OutOffset` 用于抵消静摩擦死区

---

## 通信协议

USART2（9600 8N1）连接蓝牙模块，与上位机通信。数据包以 `[` 开头、`]` 结尾，字段用英文逗号分隔。

### 上位机

配套上位机为**江协科技出品的「蓝牙串口」PC 小程序**，通过蓝牙虚拟串口与本车通信，提供两项功能：

- **波形显示**：解析小车回传的 `[plot,...]` 包，实时绘制三环 PID 积分项曲线，用于观察调参效果
- **摇杆控制**：虚拟摇杆下发 `[joystick,...]` 包，控制小车前进 / 后退 / 转弯

> 该小程序不开源，属于江协科技的配套工具。

### 接收（上位机 → 小车）

| 数据包 | 格式 | 说明 |
|---|---|---|
| 滑杆调参 | `[slider,Name,Value]` | `Name` 可取 `AngleKp` / `AngleKi` / `AngleKd` / `TurnKp` / `TurnKi` / `TurnKd` / `Offset` |
| 摇杆控制 | `[joystick,LH,LV,RH,RV]` | `LV/25.0 → SpeedPID.Target`（约 ±4 转/秒）、`RH/25.0 → TurnPID.Target` |
| 按键 | `[key,Name,Action]` | 协议已预留，当前分支为空未实现 |

### 发送（小车 → 上位机）

| 数据包 | 格式 | 说明 |
|---|---|---|
| 波形回传 | `[plot,AngleInt,SpeedInt,TurnInt]` | 三环 PID 的积分项，供上位机绘制曲线 |

> `main.c` 中还保留了一行注释掉的 `"%f,%f,%f\n"`，那是给 VOFA+（FireWater 文本协议）直接解析用的，默认未启用。

---

## 编译与烧录

### 依赖

- **Keil MDK-ARM v5**（uVision）
- **STM32F1xx Device Family Pack**（STM32F1 器件支持包）
- ST-Link 或 J-Link 驱动

### 编译

1. 打开工程 `MDK-ARM/pinghengche.uvprojx`
2. `Project → Build Target`（F7）
3. 产物输出在 `MDK-ARM/pinghengche/`：
   - `pinghengche.hex` — 可直接烧录
   - `pinghengche.axf` — 调试用带符号文件

### 烧录

- **SWD（推荐）**：ST-Link / J-Link 连接 PA13(SWDIO)、PA14(SWCLK)、GND、3V3，Keil 中 Download 即可
- **串口 ISP**：BOOT0 拉高后复位，用 PA9/PA10 经 USB-TTL 烧录，烧完 BOOT0 拉回低电平

### 修改外设配置

外设（时钟、引脚复用、定时器参数）请通过 `pinghengche.ioc` 用 STM32CubeMX 重新生成，生成后**不要覆盖** `App/`、`Hardware/`、`System/` 下的文件。

---

## 调参与标定

### 当前 PID 参数

| 环 | Kp | Ki | Kd | 输出限幅 | 积分限幅 | 控制周期 |
|---|---|---|---|---|---|---|
| 角度环 | 3.41 | 0.17 | 3.22 | ±100 | ±600 | 10 ms |
| 速度环 | 2.02 | 0.07 | 0 | ±20 | ±150 | 50 ms |
| 转向环 | 4.38 | 3.82 | 0 | ±50 | ±20 | 50 ms |

参数定义在 `Core/Src/main.c` 的 `AnglePID` / `SpeedPID` / `TurnPID` 初始化处，运行时可通过蓝牙 App 在线修改。

### 转速换算

```c
LeftSpeed  = Encoder_Get(1) / 44.0 / 0.05 / 9.27666;   // 转/秒
RightSpeed = Encoder_Get(2) / 44.0 / 0.05 / 9.27666;
```

`44` = 编码器每圈计数（11 线 × 4 倍频），`0.05` = 50 ms 采样周期，`9.27666` = 减速比。**更换电机或编码器后必须重新标定这三个系数。**

### 需要重新标定的量

- `AngleAcc += 8.1` — 加速度计解算的俯仰角零位补偿
- `GY += 3` — 陀螺仪零漂补偿
- 上表转速换算的三个系数

这些都是在实车上实测标定的经验值，换硬件后需要重新测量。

---

## 已知限制

- 平衡控制主逻辑目前写在 `Core/Src/main.c` 的 `HAL_TIM_PeriodElapsedCallback()` 内，尚未抽离到 `App/` 层
- 蓝牙 `[key,...]` 命令分支为空，功能未实现
- 转向环 `Ki` 较大（3.82），高速行进时可能出现小幅摆动，需按实车调整
- 零漂补偿与转速系数为特定硬件的标定值，非通用参数

---


### 许可说明

本项目**仅供学习与交流使用**。

---

## 致谢

- 项目整体思路与硬件方案来自 [江协科技](https://jiangxiekeji.com) 的自平衡车教程
- OLED 驱动与点阵字库基于江协科技的开源驱动改写
- 蓝牙调参 / 波形上位机为江协科技出品的「蓝牙串口」小程序
- 外设初始化由 STM32CubeMX 生成，HAL 库版权归 STMicroelectronics 所有
