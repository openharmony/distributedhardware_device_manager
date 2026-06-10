# DeviceManager 构建与测试规则

## 1. 构建入口

构建命令在 OpenHarmony 源码根目录执行，不在本仓子目录执行。本仓没有 npm、cargo、Makefile，不要发明 make / cmake 命令。

常用顶层目标：

```sh
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager_fwk
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager_test
```

## 2. Minimum checks

- C++ 改动：至少编译受影响顶层 target：`device_manager` / `device_manager_fwk` / `device_manager_test`。
- 测试改动：必须跑改动 UT；新增 UT 用例要确认加入对应 `BUILD.gn` deps。
- BUILD.gn、顶层 group、feature flag：需验证相关 target，并检查 lite 分支是否需同改。
- 改 PIN 认证：跑 `ext/pin_auth/` 关联 UT + 真实 PIN 流程冒烟，PIN 禁止明文日志。
- 改 ACL 写入 / 清理：跑关联 UT，并按 `03-verification.md` 验证持久化。

## 3. 完成定义

任务完成必须说明：

- 变更摘要与改动文件。
- 已运行的构建、测试或检查命令及结果。
- 无法运行验证时的原因和风险。
- 涉及 IPC、SDK 头、`bundle.json`、SA、权限、hisysevent 的改动需在 PR 中显式说明。
