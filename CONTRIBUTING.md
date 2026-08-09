# 贡献指南

感谢你对 Geezpai Spot 的关注！欢迎通过 Issue 和 Pull Request 参与贡献。

## 行为准则

- 保持尊重和专业
- 就事论事，对代码不对人
- 新手友好，耐心解答

## 如何贡献

### 报告 Bug

1. 搜索 [Issues](../../issues) 确认是否已有相同问题
2. 若没有，新建 Issue，包含：
   - 硬件版本（PCB 版本号）
   - 固件版本（`git log --oneline -1`）
   - 复现步骤
   - 预期行为 vs 实际行为
   - 串口日志（如有）

### 提交代码

1. **Fork** 本仓库
2. 从 `main` 分支创建特性分支：
   ```bash
   git checkout -b feature/your-feature main
   ```
3. 遵循现有代码风格（见下方）
4. 提交前确保编译通过：
   ```bash
   ./build.ps1
   ```
5. 推送到你的 Fork，然后创建 Pull Request

### Pull Request 要求

- 描述改了什么、为什么改
- 如果是新功能，包含使用示例或测试方法
- 所有 commit 通过 pre-commit hook
- 新增文件包含 Copyright + SPDX 头

## 代码风格

本项目基于 Nordic nRF Connect SDK，遵循 Zephyr RTOS 编码规范：

- **缩进**: Tab (8 字符宽)
- **行宽**: 100 字符以内
- **命名**: 公共 API 用 `app_` 前缀，`lower_snake_case`
- **返回值**: `int`，0 = 成功，负值 = errno
- **日志**: 独立模块用 `LOG_MODULE_REGISTER`，紧密集成用 `LOG_MODULE_DECLARE`
- **版权头**: 所有源文件必须包含 SPDX 标识

```c
/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
```

## 分支策略

```
main          ← 稳定版本
├── dev       ← 日常开发
├── feature/* ← 新功能
└── fix/*     ← Bug 修复
```

## 提交规范

```bash
# 提交前激活 pre-commit hook
git config core.hooksPath .githooks

# 提交信息格式
git commit -m "module: brief description"
# 例如:
#   uwb: add two-way ranging API
#   lcd: fix backlight PWM duty cycle
#   doc: update pin mapping for V1.01
```

## 联系方式

- Issue: [GitHub Issues](../../issues)
- 邮件: avery.ling@geezpai.com
