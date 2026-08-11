# DeviceManager 构建与测试规则

## 1. 构建入口

构建命令在 OpenHarmony 源码根目录执行，不在本仓子目录执行。本仓没有 npm、cargo、Makefile，不要发明 make / cmake 命令。

常用顶层目标：

```sh
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager_fwk
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager_test
```

## 2. 测试目标

### UT

UT 定义在 `test/unittest/BUILD.gn`，group 名 `unittest`，`module_out_path = "device_manager/device_manager"`。构建全量 UT：

```sh
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:test/unittest:unittest
```

构建单个 UT（以 `UTTest_device_manager_service` 为例）：

```sh
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:test/unittest:UTTest_device_manager_service
```

板上运行（产物在 `out/<product>/tests/unittest/device_manager/device_manager/`）：

```sh
hdc file send out/<product>/tests/unittest/device_manager/device_manager/UTTest_device_manager_service /data/local/tmp/
hdc shell chmod +x /data/local/tmp/UTTest_device_manager_service
hdc shell /data/local/tmp/UTTest_device_manager_service
```

### Fuzz

Fuzz 测试分布在 `test/commonfuzztest/`、`test/softbusfuzztest/`、`test/servicesfuzztest/`、`test/interfacesfuzztest/`，均使用 `ohos_fuzztest` 模板。构建示例：

```sh
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:test/commonfuzztest:PinAuthFuzzTest
```

## 3. Minimum checks

- C++ 改动：至少编译受影响顶层 target：`device_manager` / `device_manager_fwk` / `device_manager_test`。
- 测试改动：必须跑改动 UT；新增 UT 用例要确认加入对应 `BUILD.gn` deps。
- BUILD.gn、顶层 group、feature flag：需验证相关 target，并检查 lite 分支是否需同改。
- 改 PIN 认证：跑 `ext/pin_auth/` 关联 UT + 真实 PIN 流程冒烟，PIN 禁止明文日志。
- 改 ACL 写入 / 清理：跑关联 UT，并按 `02-verification.md` 验证持久化。

## 4. 静态检查

- 编译期警告即错误：部分 target 的 `cflags` 含 `-Wall -Werror`（如 `UTTest_ipc_cmd_parser_client`、`UTTest_ipc_cmd_parser_service`），所有 C++ 改动必须零警告通过编译。
- 代码格式：OpenHarmony 使用 `clang-format` 检查；改动文件后执行 `clang-format --dry-run --Werror <file>` 确认无格式差异。

## 5. API 兼容性检查

涉及公开 SDK 头、IPC code 或 `bundle.json` `inner_kits` 变更时，必须确认无 ABI break：

| 变更类型 | 检查方法 | 通过标准 |
|---|---|---|
| 公开 SDK 头签名变更 | 确认 `interfaces/inner_kits/native_cpp/` 头文件未删除或修改已有接口签名 | 无签名删改 |
| IPC code 变更 | 确认已有 IPC code 数值未变（IPC code 是 ABI） | 数值不变 |
| 错误码变更 | 确认已有错误码值未变、语义未收窄 | 值与语义不变 |
| `bundle.json` `inner_kits` 变更 | 确认未删除已登记的公开出口 | 已登记出口不删 |

## 6. 完成定义

任务完成必须说明：

- 变更摘要与改动文件。
- 已运行的构建、测试或检查命令及结果。
- 无法运行验证时的原因和风险。
- 涉及 IPC、SDK 头、`bundle.json`、SA、权限、hisysevent 的改动需在 PR 中显式说明。
