# 构建、测试与验证指南

## 1. Feature flag

`device_manager.gni` 中的关键构建开关：

| flag | 默认 | 说明 |
|---|---|---|
| `device_manager_capability` | `true` | 大部分代码和所有测试受此 gate 控制 |
| `support_jsapi` | 自动/产品配置 | 控制 JS kits、3rd interfaces，以及整个 `test/` 组 |
| `support_bluetooth` / `support_wifi` / `support_power_manager` / `support_screenlock` / `support_memmgr` / `support_msdp` / `os_account_part_exists` | 自动 | 由 `global_parts_info` 探测；新代码不要硬依赖这些子系统 |
| `device_manager_common` | 条件 | `distributed_hardware_adapter` 缺失时为 true，adapter 专用代码按此 gate |

`BUILD.gn` 在 `defined(ohos_lite)` 下另有分支。修改顶层 group 或新增组件时，两路都要检查；lite 只拉 `utils`、`services/service`、`services/implementation`、`inner_kits/native_cpp`、`softbuscache`、`test/smallunittest`。

## 2. 构建命令

构建命令在 OpenHarmony 源码根目录执行，不在本仓子目录执行。本仓没有 npm、cargo、Makefile，不要发明 make / cmake 命令。

```sh
# 编译服务侧
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager

# 编译 SDK 侧
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager_fwk

# 编译测试
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager_test
```

单 target 可用 `hb`：

```sh
hb build -T foundation/distributedhardware/device_manager/services/service:devicemanagerservice
hb build -T foundation/distributedhardware/device_manager/test/unittest:UTTest_auth_manager
```

## 3. 测试布局

所有测试都在 `test/` 下，且仅当 `support_jsapi && device_manager_capability` 同时为真时才构建。任一为假时 `device_manager_test` 为空，不要假设测试已经跑到。

| 目录 | 说明 |
|---|---|
| `test/unittest/` | GoogleTest 单元测试，命名 `UTTest_<module>.{h,cpp}`；新增用例必须加到 `test/unittest/BUILD.gn` 的 `group("unittest")` deps 列表 |
| `test/commonunittest/`、`test/softbusunittest/` | 额外单元测试套件 |
| `test/benchmarktest/` | Google Benchmark 性能测试 |
| `test/{common,interfaces,services,softbus}fuzztest/` | libFuzzer fuzz target；`fuzz_test_output_path = "device_manager/device_manager"` |
| `test/smallunittest/` | 仅 lite（LiteOS）构建使用 |

`module_out_path` 全部为 `device_manager/device_manager`，新增测试 BUILD.gn 时保持一致。

设备端跑单测：把 `out/.../tests/unittest/device_manager/device_manager/` 下的 GoogleTest 二进制 push 到 `/data/test/` 后直接执行。

## 4. Minimum checks

任何代码改动都要匹配至少一项验证：

- C++ 改动：至少跑该改动影响的顶层 target 之一：`device_manager` / `device_manager_fwk` / `device_manager_test`
- BUILD.gn 改动：用 `hb build -T <full-target>` 验证相关 target，并检查 `defined(ohos_lite)` 分支是否需同改
- `.codespec/` 改动：用 ODK 章节合同自检（`proposal.md` 6 章 / `design.md` 6 章 / `spec.md` 8 章 / `execution-plan.md` 7 章）
- 测试改动：必跑改动的 UT；新增 UT 用例还要确认已加进 `test/unittest/BUILD.gn` 的 `group("unittest")` deps
- 新增 hisysevent：先在 `hisysevent.yaml` 注册并经 `bundle.json` 引用，然后整编验证
- C++ 风格自检：单行不超过 120 字符；新增数值常量不得为魔鬼数字，可用 `rg -n '[^A-Za-z_][0-9]{2,}' <changed-files>` 辅助人工确认

## 5. Task-specific checks

| 任务类型 | 验证 |
|---|---|
| 改公开 SDK 头 | `device_manager_fwk` 全编 + lite 路径检查 + 下游 inner_kits include 路径自查 |
| 改 services/implementation | `device_manager` 全编 + `device_manager_test` 全编 + 关联 UT 设备端运行 |
| 改 IPC code | 跨进程冒烟 + 双端兼容（旧 client × 新 server / 新 client × 旧 server） |
| 改老协议路径 | `services/implementation/src/authentication/` 关联 UT 必跑 |
| 改 hisysevent | `hisysevent.yaml` 注册后全编 + 现场抓事件验证 |
| 改 BUILD.gn / 顶层 group / feature flag | lite 路径同编 + `hb build` 单 target 验证 |
| 改 PIN 认证 | `ext/pin_auth/` 关联 UT + 真实 PIN 流程冒烟，不要在日志中打印 PIN |
| 改 ACL 写入 / 清理 | 至少 `UTTest_dm_auth_message_processor` + `UTTest_device_manager_service_impl_first` 关联用例 |
| 改 `.codespec/` 文档 | ODK 4 文件章节自检 + 链接/路径完整性检查 |
| 测试-only | 改的 UT 必跑 + 至少 1 个相邻 UT |

## 6. 推包与板端验证注意事项

- 推包验证必须使用同一次 OpenHarmony 源码根目录产物，避免混用 `hb` / `build.sh` 或不同 workspace 的 so。
- 设备 reboot 后必须等待全部参与测试的板重新上线并执行 `hdc target mount` 后再推包/跑分布式测试。
- source/sink 需在同一 Wi-Fi 且保持亮屏。
- 端到端验证 DP/ACL 持久化时，把 DP RDB 主库、`-wal`、`-shm` 拉到 PC 本地只读解析；不要在 RK 板上直接跑 sqlite 查询或修改。

## 7. Done definition

任务完成必须同时满足：

- 行为已实现，命中所有验收标准或 Scenario
- 与任务匹配的 minimum + task-specific 验证已跑，或显式说明无法跑的原因
- 最终回复包含变更摘要、改动文件、验证命令与结果、剩余风险
- 没有夹带无关格式化、refactor、drive-by 改动
- 涉及 IPC / SDK 头 / inner_kits / SA / 权限 / hisysevent 的改动已在 PR 中显式说明

非平凡任务最终回复包含：

- 变更摘要
- 改动文件
- 验证命令与结果
- 兼容性 / 权限 / DFX / 跨设备影响；不涉及写“无”
- 剩余风险与 follow-up
