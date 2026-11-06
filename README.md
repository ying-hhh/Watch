# STM32 多功能智能手表（Watch）

基于 **STM32F103C8T6 + FreeRTOS** 的多功能手表项目，使用 OLED 屏显示，支持时钟、秒表、游戏、计步、水平仪等多种应用，并通过按键与语音指令进行交互。

## 项目概览

| 项目 | 说明 |
| ---- | ---- |
| 主控芯片 | STM32F103C8T6（LQFP48，Cortex-M3，主频 72 MHz） |
| 实时系统 | FreeRTOS（多任务调度） |
| HAL 库 | STM32Cube FW_F1 V1.8.6（CubeMX 6.18.1 生成） |
| 开发工具 | Keil MDK-ARM V5（`MDK-ARM/Watch.uvprojx`） |
| 显示 | OLED（I2C1，Fast 模式） |
| 传感器 | MPU6050（六轴姿态，用于计步/水平仪等） |
| 时钟源 | HSE 8 MHz ×9 = 72 MHz；RTC 使用外部 LSE 32.768 kHz |

## 功能应用

项目以 FreeRTOS 任务为核心，每个应用对应一个独立任务（见 `MDK-ARM/Application/`）：

- **App_Clock**：时钟显示（基于 RTC，可设置时间 `App_Setting`）
- **App_Stopwatch**：秒表
- **App_Flashlight**：手电筒/照明
- **App_MPU6050 / App_Pedometer**：姿态读取与计步
- **App_Gradienter**：水平仪（气泡水平仪）
- **App_Game**：小游戏
- **App_Emoji**：表情显示
- **App_Menu**：应用菜单（切换各个应用）
- **App_Voice**：语音指令识别与调度（优先级最高）

## 硬件资源分配

| 外设 | 引脚/配置 | 用途 |
| ---- | ---- | ---- |
| I2C1（Fast） | PB8 = SCL，PB9 = SDA | OLED / MPU6050 |
| USART2 | PA2 = TX，PA3 = RX | 语音模块 / 串口调试 |
| ADC1_IN0 | PA0 | 模拟量采集（如电池电压） |
| GPIO 输入（上拉） | PA4、PA6、PB1 | 按键 |
| GPIO 输出（开漏） | PB10、PB11 | 预留/I2C 类接口 |
| GPIO 输出 | PB15 | LED / 背光控制 |
| RTC | LSE 32.768 kHz | 走时 |
| TIM2 | 内部时钟 | 通用定时 |
| SWD | PA13/PA14 | 下载调试 |

## 目录结构

```
Watch/
├── Core/                  # CubeMX 生成的核心代码（main.c、外设初始化、中断）
│   ├── Inc/
│   └── Src/               # main.c、adc/i2c/rtc/tim/usart/gpio/stm32f1xx_it 等
├── Drivers/               # STM32 HAL 库与 CMSIS
├── MDK-ARM/               # Keil 工程目录
│   ├── Watch.uvprojx      # Keil 工程文件（用 Keil 打开此文件）
│   ├── Application/       # 应用层：各功能 App 任务 + Freertos_Task.c（任务创建调度）
│   ├── Hardware/          # 硬件驱动：OLED、OLED_Data（字库）、MPU6050、Key、LED、AD
│   ├── FreeRTOS/          # FreeRTOS 内核源码
│   ├── Common/            # 公共代码
│   └── System/            # 系统级支持文件
└── Watch.ioc              # CubeMX 工程配置文件
```

## 软件架构

1. `main.c` 完成 HAL 初始化与外设初始化（GPIO/USART2/I2C1/TIM2/ADC1/RTC）。
2. 调用 `FreeRTOS_Start()`（`Application/Freertos_Task.c`）：
   - 初始化 OLED、RTC、按键、MPU6050；
   - 创建各应用任务并启动调度器 `vTaskStartScheduler()`。
3. 任务间通过全局句柄 `g_current_active_task`（见 `App_config.h`）记录当前前台应用，供菜单切换与“返回”语音指令使用。
4. 语音任务（`Voice_Task`）优先级最高（3），按键任务次之（2），各应用任务为 1。

### 主要任务一览

| 任务 | 栈大小（字） | 优先级 | 说明 |
| ---- | ---- | ---- | ---- |
| Key_Task | 128 | 2 | 按键扫描（10 ms 周期） |
| Voice_Task | 128 | 3 | 语音指令识别与分发 |
| Clock_Task | 512 | 1 | 时钟显示 |
| SetTime_Task | 512 | 1 | 时间设置 |
| Menu_Task | 512 | 1 | 应用菜单 |
| StopWatch_Task | 512 | 1 | 秒表 |
| Flashlight_Task | 128 | 1 | 照明 |
| MPU6050_Task | 256 | 1 | 姿态采集 |
| Game_Task | 128 | 1 | 游戏 |
| Emoji_Task | 128 | 1 | 表情显示 |
| Gradienter_Task | 128 | 1 | 水平仪 |

## 编译与下载

1. 使用 **Keil MDK V5** 打开 `MDK-ARM/Watch.uvprojx`。
2. 直接编译（Project → Rebuild all target files）。
3. 通过 **ST-Link（SWD：PA13/PA14）** 下载到开发板。
4. 若需修改外设配置，用 **STM32CubeMX 6.18.1** 打开根目录 `Watch.ioc` 重新生成。

## 依赖

- STM32CubeF1 固件库 V1.8.6（已包含在 `Drivers/`）
- FreeRTOS 内核（已包含在 `MDK-ARM/FreeRTOS/`）
- Keil MDK-ARM V5.32 工具链

## 硬件清单（参考）

- STM32F103C8T6 最小系统板（8 MHz HSE + 32.768 kHz LSE 晶振）
- 0.96 寸 OLED（SSD1306，I2C）
- MPU6050 六轴传感器模块
- 语音识别/播报模块（USART2）
- 按键 ×3、LED 若干
- ST-Link 调试器
