# Geezpai Spot 开发指南

> 硬件: NRF_UWB_TEST V1.00 | 固件: [geezpai-spot](https://github.com/geezpai/geezpai-spot)

---

## 0. 首次配置（必须）

### 0.1 Git 认证 — 一次配置，永久有效

打开终端执行：

```bash
cd F:\work\1.git\bleuwb\nrf52840-dk_nearby

# 1. 设置作者
git config user.name "Avery Ling"
git config user.email "avery.ling@geezpai.com"

# 2. 启用 Windows 凭据管理器
git config credential.helper manager

# 3. 激活 pre-commit 敏感文件防护
git config core.hooksPath .githooks
```

首次 `git push` 会弹出 Windows 安全登录框：
- **用户名:** `geezpai`
- **密码:** 粘贴 GitHub Personal Access Token（不是 GitHub 登录密码）

Windows 会自动记住，之后不再需要输入。

### 0.2 获取敏感配置

`Kconfig.sysbuild`（含客户 Model ID / Anti-Spoofing Key）被 git 排除，需从内部渠道获取后放到工程根目录。

---

## 1. 代码更新流程

### 每次修改后

```bash
cd F:\work\1.git\bleuwb\nrf52840-dk_nearby

# 1. 查看改了什么
git status

# 2. 添加修改的文件
git add .

# 3. 提交 (描述本次改了什么)
git commit -m "描述你的修改内容"

# 4. 推送到 GitHub
git push
```

### 如果多人协作

```bash
# 先拉取别人的更新，再推送
git pull --rebase
git push
```

---

## 2. 新增功能模块

以新增一个"温湿度传感器"为例：

### 2.1 创建 Kconfig 开关

新建 `Kconfig.sensor`：

```kconfig
config APP_SENSOR
    bool "Temperature and humidity sensor"
    default y
    help
      Enable temperature and humidity sensor module.
```

在 `Kconfig` 末尾加一行 `rsource "Kconfig.sensor"`。

### 2.2 写头文件

新建 `include/app_sensor.h`：

```c
#ifndef APP_SENSOR_H__
#define APP_SENSOR_H__

int app_sensor_init(void);
float app_sensor_temp_get(void);
float app_sensor_humi_get(void);

#endif
```

### 2.3 写实现

新建 `src/sensor/app_sensor.c`：

```c
#include "app_board_pins.h"
#include "app_sensor.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(fp_fhn, LOG_LEVEL_INF);

int app_sensor_init(void)
{
    LOG_INF("Sensor module initialized");
    return 0;
}
```

新建 `src/sensor/CMakeLists.txt`：

```cmake
target_sources(app PRIVATE app_sensor.c)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR})
```

### 2.4 注册到工程

在 `CMakeLists.txt` 的 `# NORDIC SDK APP END` 前加入：

```cmake
if(CONFIG_APP_SENSOR)
  add_subdirectory(src/sensor)
endif()
```

在 `src/main.c` 的 `init_work_handle()` 中加入：

```c
if (IS_ENABLED(CONFIG_APP_SENSOR)) {
    err = app_sensor_init();
    if (err) {
        LOG_WRN("Sensor init failed (err %d), continue", err);
    }
}
```

### 2.5 提交

```bash
git add .
git commit -m "Add temperature and humidity sensor module"
git push
```

---

## 3. 构建与烧录

### 构建

```bash
cd F:\work\1.git\bleuwb\nrf52840-dk_nearby
west build -b nrf52840dk_nrf52840 -d build
```

### 烧录 (J-Link)

```bash
west flash -d build
```

### 串口查看日志

```bash
# 波特率 115200, 接到 P0.13/P0.14
```

---

## 4. 分支管理建议

```bash
main          ← 稳定版本, 可直接烧录
├── dev       ← 日常开发分支
├── feature/xxx ← 新功能开发
└── fix/xxx   ← Bug 修复
```

日常操作：

```bash
# 新功能从 main 拉分支
git checkout -b feature/sensor main

# 开发、测试完成后合并
git checkout main
git merge feature/sensor
git push

# 删掉已合并的特性分支
git branch -d feature/sensor
```

---

## 5. 关键文件速查

| 文件 | 作用 |
|------|------|
| `include/app_board_pins.h` | 所有 IO 引脚定义, 改硬件接法先改这里 |
| `configuration/boards/nrf52840dk_nrf52840.overlay` | 外设 devicetree 配置 |
| `configuration/prj.conf` | Debug 版本 Kconfig |
| `configuration/prj_release.conf` | Release 版本 Kconfig |
| `Kconfig` | 功能模块开关入口 |
| `CMakeLists.txt` | 构建脚本, 新增模块在此注册 |
| `src/main.c` | 主程序入口 |

---

> 有问题联系 avery.ling@geezpai.com
