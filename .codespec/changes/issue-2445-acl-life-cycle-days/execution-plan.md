# Execution Plan

## 输入状态

- 关联 change：`issue-2445-acl-life-cycle-days`
- 关联 capability：`binding-acl-lifetime`（新增）
- 输入文档：`proposal.md` / `design.md` / `spec.md`（同目录）
- 输入分支：`feat/trust-always-life-cycle`（本地）
- 目标版本：`OpenHarmony-6.0-Release`（来源 `proposal.md` front-matter）
- 前置条件：
  - OpenHarmony 整树已 sync 至包含 `distributed_device_profile`、`dsoftbus`、`device_auth (hichain)` 的版本
  - 本仓 `device_manager_capability = true` 且 `support_jsapi = true`（否则 test 组不生成）
  - JSON 后端：`use_nlohmann_json = true`（默认）

## AC 到 Task 追溯

| AC ID | 描述 | 关联 Task |
|-------|------|----------|
| AC-1 | 永久可信 + 合法天数：校验通过 → ACL 写入整数 | TASK-1, TASK-2, TASK-3, TASK-4 |
| AC-2 | 非永久可信 + 正数天数：入口拒绝 | TASK-2 |
| AC-3 | 缺失/-1：校验通过 → 不写入 → 清理 365 天 | TASK-2, TASK-3, TASK-4, TASK-5 |
| AC-4 | 清理按 ACL 自带天数判过期；异常 fallback 365 天 | TASK-5 |
| AC-5 | 跨版本双向兼容 | TASK-2, TASK-4, TASK-5（fallback 路径） |

Requirement → Task 映射：

| Requirement | 主要 Task | 验证 Task |
|-------------|----------|----------|
| R-1（业务声明字段） | TASK-1 | TASK-2 间接覆盖 |
| R-2（入口交叉校验） | TASK-2 | TASK-2 子项 2.4 |
| R-3（绑定写入 AccessControlProfile.extraData） | TASK-3, TASK-4 | TASK-3 子项 3.4 + TASK-4 子项 4.5 |
| R-4（清理读取） | TASK-5 | TASK-5 子项 5.4 |
| R-5（向后兼容） | 散落于 TASK-2/3/4/5 的 fallback 用例 | TASK-6 端到端 |

## 实现边界

**Must do（本 change 范围）**：

- 仅修改 `services/service`、`services/implementation/authentication_v2`、`services/implementation/device_manager_service_impl`、`common/{include,src}/dm_constants.{h,cpp}`
- 仅修改客体（sink）侧逻辑
- 仅修改 authentication_v2 协议路径
- 测试新增在 `test/unittest/` 现有 4 个文件中追加用例，**不新增** UT 二进制 / 不改 `test/unittest/BUILD.gn`

**Must NOT do（明确不做）**：

- 不修改 `interfaces/inner_kits/native_cpp` 公有头文件
- 不修改 `common/include/device_manager_ipc_interface_code.h`（IPC ABI 稳定）
- 不修改 `bundle.json` / `sa_profile/` / `permission/` / `hisysevent.yaml`
- 不修改 `services/implementation/src/authentication/dm_auth_manager.cpp`（老协议路径）
- 不修改 `3rd/` 第三方栈
- 不修改主端（source）逻辑、`accesser.extraData` / `accessee.extraData`
- 不修改 DP 数据库 schema
- 不实现已有 ACL 回填工具
- 不在 ACL 中写入冗余字段（如 `EXPIRE_TIMESTAMP`）

## Task 列表

### TASK-1 — 常量与基础设施

- [ ] 1.1 在 `common/include/dm_constants.h` 声明 `ACL_LIFE_CYCLE_DAYS` (const char*)、`ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED` (-1)、`ACL_LIFE_CYCLE_DAYS_MIN` (1)、`ACL_LIFE_CYCLE_DAYS_MAX` (3650)
- [ ] 1.2 在 `common/src/dm_constants.cpp` 提供常量定义
- [ ] 1.3 构建 `devicemanagerutils` 通过

### TASK-2 — 入口校验（service 进程）

- [ ] 2.1 在 `services/service/include/device_manager_service.h` 声明 private `IsAclLifeCycleDaysValid(int authType, const std::string &extraInfo)`
- [ ] 2.2 在 `services/service/src/device_manager_service.cpp` 实现 `IsAclLifeCycleDaysValid`：`JsonObject::Parse` → `IsDiscarded` 短路 → `IsNumberInteger` → `Get<int32_t>` → 范围与 authType 交叉判定
- [ ] 2.3 在 `IsImportAuthInfoValid` 调用链中接入 `IsAclLifeCycleDaysValid`，拒绝时返回 `ERR_DM_INPUT_PARA_INVALID`
- [ ] 2.4 在 `test/unittest/UTTest_device_manager_service_three.cpp` 新增 10 个用例覆盖完整校验矩阵（含 mock setup）

### TASK-3 — context 解析（authentication_v2 sink 侧）

- [ ] 3.1 在 `services/implementation/include/authentication_v2/dm_auth_context.h` 为 `DmAuthContext` 新增 `int32_t aclLifeCycleDays{ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED}` 字段
- [ ] 3.2 在 `services/implementation/src/authentication_v2/auth_stages/auth_confirm.cpp` 匿名命名空间新增 `ExtractAclLifeCycleDays(const std::string &)` helper：解析 JSON → 合法返回天数，否则 `NOT_CONFIGURED`
- [ ] 3.3 在 `ProcessImportAuthInfo` 末尾调用 helper 并写入 `context->aclLifeCycleDays`
- [ ] 3.4 在 `test/unittest/UTTest_auth_confirm.cpp` 新增 5 个用例覆盖：合法/哨兵/缺失/malformed/字符串类型

### TASK-4 — ACL 写入（authentication_v2 sink 侧）

- [ ] 4.1 在 `PutNonLnnAclProfile` 中，当 `confirmOperation == ALLOW_AUTH_ALWAYS && aclLifeCycleDays ∈ [MIN, MAX]` 时追加 `extraData[ACL_LIFE_CYCLE_DAYS] = context->aclLifeCycleDays`，然后 `profile.SetExtraData(extraData.Dump())`
- [ ] 4.2 确保 `SetTransmitAccessControlList` 不再写入 `ACL_LIFE_CYCLE_DAYS` 到 accessee/accesser ExtraData（只写纯实体身份信息 `context->accessee.extraInfo`）
- [ ] 4.3 删除已废弃的 `BuildAccesseeExtraDataWithAclDays` 匿名辅助函数
- [ ] 4.4 确保不写 accesser/accessee 侧 ExtraData 的关系级属性
- [ ] 4.5 在 `test/unittest/UTTest_dm_auth_message_processor.cpp` 新增 5 个用例：ACL extraData 写入整数 / 非永久不写 / sentinel 不写 / accessee.extraData 无 key / malformed base 不崩溃

### TASK-5 — 清理消费（service impl）

- [ ] 5.1 在 `services/implementation/include/device_manager_service_impl.h` 为 `DeviceManagerServiceImpl` 声明 private `int64_t GetAclAllowSeconds(const AccessControlProfile &)`
- [ ] 5.2 在 `services/implementation/src/device_manager_service_impl.cpp` 实现 `GetAclAllowSeconds`：解析 `AccessControlProfile.extraData` JSON，合法整数 ∈ [MIN, MAX] 返回 `days*86400`，否则返回 `MAX_ALWAYS_ALLOW_SECONDS`
- [ ] 5.3 在 `DeleteAlwaysAllowTimeOut` 中将原硬编码 `MAX_ALWAYS_ALLOW_SECONDS` 替换为 `GetAclAllowSeconds(item)` 调用
- [ ] 5.4 在 `test/unittest/UTTest_device_manager_service_impl_first.cpp` 新增 6 个用例覆盖完整 contract 表（空/无 key/合法/超界/类型错/malformed fallback）

### TASK-6 — 端到端验证

- [ ] 6.1 全组件冒烟构建：`./build.sh --product-name rk3568 --build-target foundation/distributedhardware/device_manager:device_manager`
- [ ] 6.2 4 个 UT 二进制构建：`UTTest_device_manager_service_three` / `UTTest_auth_confirm` / `UTTest_dm_auth_message_processor` / `UTTest_device_manager_service_impl_first`
- [ ] 6.3 设备上 GoogleTest 运行（hdc push 后执行）确认 26 个用例全 PASS
- [ ] 6.4 端到端验证：永久可信 + 1095 天配置 → 由归档脚本 `https://gitcode.com/hwzhangchuang/OH_DeviceManager_SDD_AI_TEST_CODE/blob/main/issue-2445-acl-life-cycle-days/xdevice/run_test.py` 拉取 sink 板 DP RDB 到本地，并断言 ACL `extraData` 含整数 `"ACL_LIFE_CYCLE_DAYS": 1095`（**不**走 DP SDK `GetAllAccessControlProfile`，DP 进程名白名单仅允许 `device_manager`/`softbus_server`）

## 每个 Task 的完成判据

| Task | 完成判据（DoD） |
|------|----------------|
| TASK-1 | 4 个常量声明 + 定义齐全；`devicemanagerutils` 编译通过；无新增 lint 警告 |
| TASK-2 | `IsAclLifeCycleDaysValid` 实现完成；`IsImportAuthInfoValid` 调用接入；`UTTest_device_manager_service_three` 新增 10 用例全 PASS；覆盖 spec.md 校验矩阵 5 行 |
| TASK-3 | `DmAuthContext.aclLifeCycleDays` 字段新增、默认值正确；`ProcessImportAuthInfo` 中赋值生效；`UTTest_auth_confirm` 新增 5 用例全 PASS |
| TASK-4 | `PutNonLnnAclProfile` 在 `extraData[ACL_LIFE_CYCLE_DAYS]` 追加写入；`SetTransmitAccessControlList` 不再写关系级属性到 accessee ExtraData；`BuildAccesseeExtraDataWithAclDays` 已删除；`UTTest_dm_auth_message_processor` 新增 5 用例全 PASS；malformed base 不 abort |
| TASK-5 | `GetAclAllowSeconds` 实现完成；`DeleteAlwaysAllowTimeOut` 调用替换硬编码；`UTTest_device_manager_service_impl_first` 新增 6 用例全 PASS（含 fallback 路径） |
| TASK-6 | 全组件冒烟构建通过；4 个 UT 二进制构建通过；设备端 26 个用例全 PASS；手工 1095 天端到端验证通过 |

## 验证命令或验证方式

构建（在 OpenHarmony 根目录执行）：

```bash
./build.sh --product-name rk3568 --build-target foundation/distributedhardware/device_manager:device_manager
./build.sh --product-name rk3568 --build-target foundation/distributedhardware/device_manager:device_manager_test
```

或用 `hb` 单 target 验证：

```bash
hb build -T foundation/distributedhardware/device_manager/test/unittest:UTTest_device_manager_service_three
hb build -T foundation/distributedhardware/device_manager/test/unittest:UTTest_auth_confirm
hb build -T foundation/distributedhardware/device_manager/test/unittest:UTTest_dm_auth_message_processor
hb build -T foundation/distributedhardware/device_manager/test/unittest:UTTest_device_manager_service_impl_first
```

设备端运行（GoogleTest 二进制位于 `out/<product>/tests/unittest/device_manager/device_manager/`）：

```bash
hdc file send out/rk3568/tests/unittest/device_manager/device_manager/UTTest_device_manager_service_three /data/test/
hdc shell /data/test/UTTest_device_manager_service_three --gtest_filter=*AclLifeCycleDays*
# 对另外 3 个 UT 二进制重复
```

端到端验证由独立测试仓自动化脚本覆盖：
`https://gitcode.com/hwzhangchuang/OH_DeviceManager_SDD_AI_TEST_CODE/blob/main/issue-2445-acl-life-cycle-days`

1. 构造业务调用 `ImportAuthInfo`，`authType=6`、`extraInfo` 含 `"ACL_LIFE_CYCLE_DAYS": 1095`
2. 完成绑定后由 PC 端 `xdevice/lib/acl_db_verifier.py` 通过 `hdc file recv -r /data/service/el1/public/database/distributed_device_profile_service/acl_db` 把 DP RDB 整目录拉到本机，再用 Python 内置 `sqlite3` 打开 `*.db`，动态定位 ACL 表中的 `AccessControlProfile.extraData` 对应列，校验 JSON 含整数 `"ACL_LIFE_CYCLE_DAYS": 1095`。**不走** DP SDK `GetAllAccessControlProfile`——该 API 在 DP 的 `permission.json` 中限定为 `device_manager`/`softbus_server` 进程名；测试 demo 通过 `EnsureCollaborationFwkToken()` 可把 self token 的 `processName` 覆写为 `CollaborationFwk` 以过 DM 白名单，但仍不在 DP 白名单内，无权调用 DP API
3. 通过模拟时间或临时调小 `MAX_ALWAYS_ALLOW_SECONDS` 常量再走 `DeleteAlwaysAllowTimeOut`，确认按 1095 天而非 365 天判过期

## 代码范围映射

| Task | 文件路径 | 关键符号 |
|------|---------|---------|
| TASK-1 | `common/include/dm_constants.h`, `common/src/dm_constants.cpp` | `ACL_LIFE_CYCLE_DAYS`, `ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED`, `ACL_LIFE_CYCLE_DAYS_MIN`, `ACL_LIFE_CYCLE_DAYS_MAX` |
| TASK-2 | `services/service/include/device_manager_service.h`, `services/service/src/device_manager_service.cpp` | `DeviceManagerService::IsAclLifeCycleDaysValid`, `DeviceManagerService::IsImportAuthInfoValid` |
| TASK-3 | `services/implementation/include/authentication_v2/dm_auth_context.h`, `services/implementation/src/authentication_v2/auth_stages/auth_confirm.cpp` | `DmAuthContext::aclLifeCycleDays`, 匿名 `ExtractAclLifeCycleDays`, `ProcessImportAuthInfo` |
| TASK-4 | `services/implementation/src/authentication_v2/dm_auth_message_processor.cpp` | `DmAuthMessageProcessor::PutNonLnnAclProfile`（追加 `ACL_LIFE_CYCLE_DAYS` 到 `profile.extraData`）, `SetTransmitAccessControlList`（去除关系级写入） |
| TASK-5 | `services/implementation/include/device_manager_service_impl.h`, `services/implementation/src/device_manager_service_impl.cpp` | `DeviceManagerServiceImpl::GetAclAllowSeconds`, `DeviceManagerServiceImpl::DeleteAlwaysAllowTimeOut` |
| 测试 (随 TASK-2..5) | `test/unittest/UTTest_device_manager_service_three.cpp` | +10 cases |
| 测试 (随 TASK-3) | `test/unittest/UTTest_auth_confirm.cpp` | +5 cases |
| 测试 (随 TASK-4) | `test/unittest/UTTest_dm_auth_message_processor.cpp` | +5 cases |
| 测试 (随 TASK-5) | `test/unittest/UTTest_device_manager_service_impl_first.cpp` | +6 cases |

Commit message 规范（参考 ODK `contracts.md` §Git 管理约定）：

```
feat(dm): add configurable ACL lifetime for trust-always binding (#2445)

Task: TASK-1, TASK-2, TASK-3, TASK-4, TASK-5, TASK-6
```
