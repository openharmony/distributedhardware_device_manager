# Design

## 需求基线摘要

来源：`proposal.md` + `spec.md`（capability `binding-acl-lifetime` 的 5 个 ADDED Requirements）

承接的核心需求：

| 需求 ID | 简述 | 主要落点 |
|--------|------|---------|
| R-1 | 业务通过 `DmAuthInfo.extraInfo` 整数 key `ACL_LIFE_CYCLE_DAYS ∈ [1, 3650]` 配置永久可信 ACL 保留时长 | 业务接口契约 |
| R-2 | `authType × ACL_LIFE_CYCLE_DAYS` 交叉校验（非 6 + 正数 → 拒绝） | `IsImportAuthInfoValid` |
| R-3 | 永久可信绑定时将保留时长写入 ACL `AccessControlProfile.extraData`（不写 accesser 侧） | ACL 写入流程 |
| R-4 | 客体侧周期清理按 ACL 自带保留时长判过期，异常 fallback 365 天 | `DeleteAlwaysAllowTimeOut` + `GetAclAllowSeconds` |
| R-5 | 未配置时行为完全等价于改前（向后兼容） | 全链路 fallback 路径 |

`target_release` 来源：`proposal.md` front-matter（`OpenHarmony-6.0-Release`），本文不复制。

## 方案概述

DM 客体侧"永久可信"ACL 保留时长当前硬编码为 365 天（`MAX_ALWAYS_ALLOW_SECONDS = 31536000`），见 `services/implementation/src/device_manager_service_impl.cpp` 中 `DeleteAlwaysAllowTimeOut`。业务方在 `ImportAuthInfo` 时无法携带"业务自定义保留时长"参数；同时客体侧绑定流程（`authentication_v2`）也没有把 `LocalServiceInfo.extraInfo` 透传到 ACL `AccessControlProfile.extraData` 的链路。本设计在客体侧打通该链路并保持向后兼容。

**Goals**

- 业务方可通过 `DmAuthInfo.extraInfo` JSON 中的整数 key `ACL_LIFE_CYCLE_DAYS`（[1, 3650]）配置永久可信 ACL 自定义保留时长
- 仅在最终判定为永久可信时生效（用户弹框选择 `ALLOW_AUTH_ALWAYS` 或客体侧匹配 `LocalServiceInfo.authType==6`，OR 语义）
- 未配置时永久可信 ACL 保留时长仍为 365 天，行为完全等价于改前

**Non-Goals**

- 不改 SDK 接口签名 / IPC code / DP schema / `bundle.json` inner_kits
- 不改主端、老协议（`authentication/dm_auth_manager.cpp`）、3rd 栈
- 不实现"已有 ACL 回填"：业务改规则后只影响新建 ACL
- 不实现主端感知：主端 ACL 不带该字段，主端清理逻辑不感知
- PIN 多次绑定 / `ClearAuthInfo` / 组合策略均不在本 change 范围

### 方案要点

引入 `ACL_LIFE_CYCLE_DAYS` 作为 `DmAuthInfo.extraInfo` JSON 中的整数 key，**约束仅在最终判定为永久可信时生效**：

1. **入口校验**（service 进程）：`IsImportAuthInfoValid` 解析 `dmAuthInfo.extraInfo` JSON，对 `ACL_LIFE_CYCLE_DAYS` 做 `authType` 交叉校验
2. **绑定流转**（authentication_v2 sink 侧）：从 DP `LocalServiceInfo.extraInfo` 解析到 `DmAuthContext.aclLifeCycleDays` 字段，在 ACL 写入流程写入 `AccessControlProfile.extraData` JSON
3. **清理消费**（service impl）：`DeleteAlwaysAllowTimeOut` 通过新 helper `GetAclAllowSeconds` 读取 ACL 自带的 `ACL_LIFE_CYCLE_DAYS`，缺失/非法时 fallback `MAX_ALWAYS_ALLOW_SECONDS`（365 天）

**关键判定**：永久可信判定走 OR 语义——用户在弹框选 6 OR 客体侧匹配到的 `LocalServiceInfo.authType==6`，任一为真即生效。这两条路径在现有 `auth_confirm.cpp:1306-1317` 中天然汇聚到 `context->confirmOperation == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS`，写入判定只需读 `confirmOperation` 即可。

**端到端流转链路**：

| 业务步骤 | 落地点 |
|---------|--------|
| ①业务构建 `DmAuthInfo` 并配置 `extraInfo.ACL_LIFE_CYCLE_DAYS` | 业务接口契约，无代码改动 |
| ②`ImportAuthInfo` 入口接收 | `IsImportAuthInfoValid` 入口校验（TASK-002） |
| ③DP 持久化 `LocalServiceInfo` | 复用已有存储路径，无需修改 |
| ④绑定中 DM 查询有效期策略 | `ProcessImportAuthInfo` 解析 `srvInfo.extraInfo`（TASK-003） |
| ⑤DM 计算 ACL 过期时间 / 写入 ACL 扩展字段 | `PutNonLnnAclProfile` 在 `extraData[ACL_LIFE_CYCLE_DAYS]` 写入 `AccessControlProfile.extraData`（TASK-004） |
| ⑥客体侧 ACL 过期检查 | `GetAclAllowSeconds` + `DeleteAlwaysAllowTimeOut`（TASK-005） |

> 过期时间戳由客体侧清理逻辑在运行时由 `aclCreateTime + days*86400` 计算得出，避免快照值与系统时间漂移的不一致问题。ACL `AccessControlProfile.extraData` 上仅持久化 `ACL_LIFE_CYCLE_DAYS` 一个整数字段，不冗余写入 `EXPIRE_TIMESTAMP`。

## 模块影响

| 子系统 | 仓库 | 模块 | 影响类型 |
|--------|------|------|---------|
| distributedhardware | foundation/distributedhardware/device_manager | common (常量) | 新增 4 个常量 |
| distributedhardware | foundation/distributedhardware/device_manager | services/service | `IsImportAuthInfoValid` 扩展校验 |
| distributedhardware | foundation/distributedhardware/device_manager | services/implementation/authentication_v2 | `DmAuthContext` 新增字段 + 解析 + 写入 |
| distributedhardware | foundation/distributedhardware/device_manager | services/implementation (清理) | `DeleteAlwaysAllowTimeOut` 引用新 helper |

## 关键设计决策

| ID | 问题 | 选择 | 备选 | 理由 |
|----|------|------|------|------|
| D-001 | 配置承载方式 | 复用 `DmAuthInfo.extraInfo` 中新增 `ACL_LIFE_CYCLE_DAYS` key | 新增 `DmAuthInfo` 顶层字段 `TRUST_ALWAYS_LIFE_CYCLE_DAYS` | 复用 extraInfo 不改 SDK 结构体；顶层字段会扩散 IPC ABI 变更 |
| D-002 | JSON value 类型（业务输入） | **整数** (`730`) | 字符串 (`"730"`) | 整数避免手写 digit parser；JsonObject 提供 `IsNumberInteger`/`Get<int32_t>` 原生支持 |
| D-003 | `LocalServiceInfo` 匹配主键 | 沿用现有 `bundleName + pinExchangeType` | 改为 `tokenId + bindType` | 已有索引零改动；`tokenId` 已经写在 `LocalServiceInfo.extraInfo` 中可辅助识别 |
| D-004 | "永久可信"判定时机 | 走已有 `confirmOperation == ALLOW_AUTH_ALWAYS` 汇聚点 | 分别检查用户选择和预置规则 | 已有代码已自然汇聚；不引入新分支 |
| D-005 | ACL 上自定义保留时长的存储位置 | `AccessControlProfile.extraData` JSON | DP 现成的 `ValidPeriod` 字段 | `ExtraData` 为 DM 已有扩展承载位；`ValidPeriod` 在 DM 当前未使用，行为不明 |
| D-006 | ACL ExtraData 中 value 类型 | **整数** | 字符串 | DM 内部已规范化值，整数更紧凑；reader 也用 `IsNumberInteger` 校验 |
| D-007 | 是否写入 accesser/accessee 侧 ExtraData | **否**——该字段属于关系级属性，只写 `AccessControlProfile.extraData` | 写入 `accessee.ExtraData` | `accesser`/`accessee` 只装实体身份信息；同一实体可能关联多条 ACL，写在实体侧会多行覆盖 |
| D-008 | `aclLifeCycleDays` context 字段默认值 | `ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED` (-1) | `0` | 哨兵值更明确语义；与 `0` (合法天数 0 应被拒) 区分 |
| D-009 | `JsonObject::Parse` 失败处理 | 显式 `IsDiscarded()` 短路：base 合法时 `Duplicate(parsed)`，malformed 时退化为空 JSON 重新写入 | 直接 subscript | 代码库构建带 `-fno-exceptions`；discarded 状态的 subscript 会抛 → abort |
| D-010 | 是否在 ACL ExtraData 中冗余写 `EXPIRE_TIMESTAMP` | **不写** | 写入快照过期时间戳 | 过期时间戳由清理路径运行时由 `aclCreateTime + days*86400` 推算，避免快照值在跨设备/跨时区/手动改表场景下与实际过期点不一致；只持久化输入参数 `ACL_LIFE_CYCLE_DAYS` 一个权威字段 |
| D-011 | `ExportAuthInfo`/`ImportAuthInfo` 在开源镜像上的返回值 | **返回 `DM_OK`** | 返回 `ERR_DM_UNSUPPORTED_METHOD` | `DEVICE_MANAGER_COMMON_FLAG` 定义时，`DeviceManagerService::ImportAuthInfo`/`ExportAuthInfo` IPC 入口跳过闭源第二段并直接返回 `DM_OK`（`device_manager_service.cpp:1823-1830` / `:1866-1873`）。这样 `ExportAuthInfo` 的 IPC stub 才会把 `DmAuthInfo`（含 PIN）回传给 SDK 调用方。`ERR_DM_UNSUPPORTED_METHOD` 仅用于闭源/商用版本 resident so 缺失或加载异常等场景 |

## 风险与兼容性

- **风险 1**：业务方在 `authType!=6` 时配置了合法天数 → `IsImportAuthInfoValid` 拒绝导入 → **Mitigation**：明确文档；测试矩阵覆盖。
- **风险 2**：业务后续走"重置策略"流程时已建 ACL 仍带 `ACL_LIFE_CYCLE_DAYS` → **Mitigation**：保留 ACL 上的快照值，业务想重置需走解绑 + 重新绑定流程。
- **兼容性**：未配置时行为完全等同于改前；旧版本服务读到带 `ACL_LIFE_CYCLE_DAYS` 的 ACL 时会忽略该字段，按 `MAX_ALWAYS_ALLOW_SECONDS` 清理（早 fallback 行为安全）。
- **`-fno-exceptions` 注意**：所有 JSON 解析路径必须 `IsDiscarded` 短路；不可使用 `std::stoi`/`try-catch`；JsonObject 副本必须经 `Duplicate()` 而非 `operator=`。
- **默认值行为**：

  | 策略 key | 未配置时默认 | 落点 |
  |---------|------------|------|
  | `ACL_LIFE_CYCLE_DAYS` | 365 天 | `GetAclAllowSeconds` 缺失 key / 非整数 / 超界 → `MAX_ALWAYS_ALLOW_SECONDS = 31536000` 秒（=365 天） |

- **存储位置**：策略快照写入 DP 服务的 ACL 可扩展字段（`AccessControlProfile.extraData` JSON），不修改 DP 数据库 schema，新字段名为 `ACL_LIFE_CYCLE_DAYS`（不引入 `EXPIRE_TIMESTAMP`，理由见 D-010）。

### 部署与回滚

- 部署：纯增量改动；旧版本服务读新 ACL 自动忽略 `ACL_LIFE_CYCLE_DAYS` 字段并 fallback 到 365 天，无 schema 迁移
- 回滚：回退到上一版本服务后，已写入 ACL `AccessControlProfile.extraData` 的 `ACL_LIFE_CYCLE_DAYS` 被忽略；行为退化为统一 365 天保留期，无数据损坏
- 业务侧：调用方需将原 `extraInfo` 中（若已有）字符串类型的 `"ACL_LIFE_CYCLE_DAYS"` 改为整数；未使用该 key 的业务零修改

### Open Questions

- 是否需要在后续 change 中提供"批量回填"工具，使现有 ACL 也能应用新规则？当前判定：不需要，等业务真正提出诉求后再做。
- `ACL_LIFE_CYCLE_DAYS_MAX = 3650`（10 年）是否过于宽松？当前判定：保留 10 年上限以覆盖长生命周期 IoT 设备；如安全策略收紧可后续调小常量。

## 验证思路

- 单元测试覆盖 4 个层面：入口校验（10 用例）、context 解析（5 用例）、ACL 写入（5 用例）、清理读取（6 用例），共 26 用例
- 关键场景：
  - 永久可信 + 合法天数 → 写入成功 + 清理按自定义天数判过期
  - 永久可信 + 字段缺失/-1 → 不写 ExtraData + 清理 fallback 365 天
  - 一次性可信 + 任意天数 → 拒绝导入
  - 永久可信 + 用户选择 vs 预置规则 → OR 语义两路径都生效
  - malformed JSON、超界值、非整数类型 → 全部安全 fallback
- 全组件冒烟构建（`./build.sh --product-name rk3568`）+ 4 个 UT 二进制链接验证
- 设备上 GoogleTest 运行验证（待合入前由开发者完成）

## 分布式自动化测试设计

### 目标
在两台 RK3568 开发板（同 Wi-Fi 热点）上，由 xdevice 端到端验证 PR 引入的 `ACL_LIFE_CYCLE_DAYS` 在 sink 端写入 LocalServiceInfo 后，bind 流程结束能正确持久化到 ACL `AccessControlProfile.extraData`。

### 设备角色
- rk1：source（主控端，发起绑定）
- rk2：sink（被控端，注册 LocalServiceInfo 与 PIN）

### 整体方案
1. demo 源码不随本 PR 上传，统一归档在独立测试代码仓：`https://gitcode.com/hwzhangchuang/OH_DeviceManager_SDD_AI_TEST_CODE/blob/main/issue-2445-acl-life-cycle-days`。复现时按归档仓 README 将该目录临时挂入 `device_manager` 编译图后参与子部件编译
2. 走新接口路径 `ExportAuthInfo` / `ImportAuthInfo`（详见 `docs/04-workflows/08-import-pin-no-popup-bind.md`）规避 PIN 安全弹框
3. 两端 demo 二进制名分别为 `CollaborationFwk_sink` / `CollaborationFwk_source`（仅用于磁盘 / `argv[0]` 区分），在 `main()` 起始位置由 `common/sec_token_kit.cpp::EnsureCollaborationFwkToken()` 通过 `GetAccessTokenId + SetSelfTokenID + ReloadNativeTokenInfo` 三件套把本进程的 self token 覆写为 `processName="CollaborationFwk"`、`APL=system_core` 的 native token，使所有发往 DM 的 IPC caller `processName` 等值匹配 `AUTH_CODE_WHITE_LIST`（`services/service/src/permission/standard/permission_manager.cpp::CheckProcessNameValidOnAuthCode`，**精确等值，非前缀**）。同台板上 `AccessTokenKit::GetNativeTokenId("CollaborationFwk")` 也会返回 demo 自己注册的 tokenId，因此 sink 端 `auth_negotiate.cpp:149` 的 `GetNativeTokenIdByName` 直接命中 demo 的 token（归档仓 `p2_probe` 已在 RK3568 上端到端验证过）
4. `Init(pkgName)` 与 `DmAuthInfo.pinConsumerPkgName` / `bizSrcPkgName` / `bizSinkPkgName` / `regPkgName` 全部取 `"CollaborationFwk"`：与进程身份等值匹配 `AUTH_CODE_WHITE_LIST`，同时满足 sink 端 `AuthSinkConfirmState` 走 `BUNDLE_NAME_COLLABORATION_FWK` 兼容分支语义
5. **sink 双段调用**：先 `ExportAuthInfo(authInfo, 6)` 让 DM 生成 6 位 PIN（PIN 由 DM 写回 `authInfo.pinCode`，业务自生成 PIN 路径已日落不建议用），然后 sink 立即在本机调一次 `ImportAuthInfo(authInfo)`（同一 PIN + `extraInfo={"ACL_LIFE_CYCLE_DAYS":1095}`）触发开源段 `DeviceManagerServiceImpl::ImportAuthInfo`（`services/implementation/src/device_manager_service_impl.cpp:3518`）把 `LocalServiceInfo`（含 `extraInfo`）落到 sink 的 DP RDB。开源镜像上 `ExportAuthInfo`/`ImportAuthInfo` 应返回 `DM_OK`，见本设计 D-011
6. xdevice 把 sink 输出的 PIN 通过 `hdc file recv/send` 带外传给 source
7. source 用同 `pinConsumerPkgName` / `pinExchangeType=IMPORT_AUTH_CODE(5)` / `authType=TRUST_ALWAYS(6)` 调 `ImportAuthInfo(authInfo)`，然后 `StartDiscovering` 拿到 sink 的 `wifiIp/wifiPort`，调 `BindTarget`
8. bind 流转中 **sink 状态机** `AuthSinkConfirmState::Action`（开源 `auth_confirm.cpp`）通过 `GetLocalServiceInfoByBundleNameAndPinExchangeType("CollaborationFwk", authType, srvInfo)` 命中 sink 在 ⑤ 写入的 `LocalServiceInfo`，`ProcessImportAuthInfo` → `ExtractAclLifeCycleDays(srvInfo.GetExtraInfo())` 写 `context->aclLifeCycleDays`，最终在 `PutNonLnnAclProfile` 阶段写入 ACL `AccessControlProfile.extraData`
9. bind 完成后由 **PC 端 xdevice** 通过 `hdc file recv -r` 把 sink 板上的 DP RDB 整目录拉到本机，再用 Python `sqlite3` 本地解析；sink 二进制**不**调 DP SDK

> **为什么 sink 不直接调 DP SDK 验证**：`DistributedDeviceProfileClient::GetAllAccessControlProfile` 在 `foundation/deviceprofile/device_info_manager/permission/permission.json` 中限定调用进程名仅为 `device_manager` / `softbus_server`。本测试 demo 即使把 self token 的 `processName` 覆写为 `CollaborationFwk`，仍然不是 DP 白名单中的 `device_manager` / `softbus_server`，DP 仍会拒绝。改为 PC 端拉数据库本地解析后，sink 二进制也不再需要 DP SDK 依赖。

### 测试代码归档路径

为满足项目归档要求，自动化测试 demo 源码不随本 PR 提交，独立存放在：

`https://gitcode.com/hwzhangchuang/OH_DeviceManager_SDD_AI_TEST_CODE/blob/main/issue-2445-acl-life-cycle-days`

复现本需求设计和测试时，从上述归档仓检出 `issue-2445-acl-life-cycle-days` 目录，并按归档仓 README 的 BUILD.gn 接入方式挂入本地编译。PR 正式代码只保留设计说明与业务实现，不携带 demo 源码。

### 目录结构
归档仓 `issue-2445-acl-life-cycle-days/` 目录结构：

```
issue-2445-acl-life-cycle-days/
├── BUILD.gn                    # 顶层 group("testcode_issue2445")，gated
├── README.md                   # 用例运行说明
├── common/                     # 测试用具名常量、JSON 助手、anonymize、self-token 覆写
│   ├── test_constants.h
│   ├── json_helpers.{h,cpp}
│   ├── anonymize.{h,cpp}
│   └── sec_token_kit.{h,cpp}   # EnsureCollaborationFwkToken() helper
├── sink/                       # ohos_executable，output_name=CollaborationFwk_sink
│   ├── BUILD.gn
│   └── sink_demo.cpp           # 单一职责：EnsureCollaborationFwkToken + ExportAuthInfo + 写 PIN 文件
├── source/                     # ohos_executable，output_name=CollaborationFwk_source
│   ├── BUILD.gn
│   └── source_demo.cpp         # EnsureCollaborationFwkToken + ImportAuthInfo → Discover → BindTarget
├── p2_probe/                   # 权限/Token 自检 ohos_executable（非测试主链路）
│   ├── BUILD.gn
│   └── probe.cpp               # GetAccessTokenId/SetSelfTokenID/GetNativeTokenId 三件套，输出一行 JSON
└── xdevice/                    # 调度脚本（不参与 OH 编译）
    ├── run_test.py             # 全流程编排
    ├── config/user_config.xml
    └── lib/
        ├── hdc_helper.py
        ├── report_writer.py
        └── acl_db_verifier.py  # PC 端拉 DP RDB + sqlite3 解析 + 断言
```

### 编译挂载
- 归档仓 `BUILD.gn` 聚合 sink/source/p2_probe 三个 `ohos_executable`
- 本地复现时推荐按归档仓 README 将 `issue-2445-acl-life-cycle-days` 临时挂入本仓 `device_manager` 编译目标，让默认 `device_manager` 编译目标同时产出 demo；该 BUILD.gn 挂载改动仅用于本地验证，不随 PR 提交
- sink/source/p2_probe BUILD.gn 通过 `external_deps += [ "access_token:libaccesstoken_sdk", "access_token:libnativetoken_shared", "access_token:libtokensetproc_shared" ]` 引入三件套 SDK；必须使用 `_shared` 动态库版本，否则静态 `libnativetoken.a` 会因 `Restorecon` 未解析链接失败
- sink BUILD.gn **不**依赖 `device_info_manager:distributed_device_profile_*`（即便 self token 覆写为 `CollaborationFwk`，DP 仍只放行 `device_manager`/`softbus_server`，避免误导）
- 不引入对 `.codespec` 目录的反向耦合；其他子目录不感知归档测试代码
- **p2_probe** 仅用于权限/Token 自检：当后续 OH 版本调整 `access_token` 行为时，30 秒可复测 `EnsureCollaborationFwkToken()` 的两条核心断言（P1：`SetSelfTokenID` 成功；P2：`GetNativeTokenId("CollaborationFwk")` 命中 demo 自己的 token），见归档仓 `issue-2445-acl-life-cycle-days/p2_probe/`

### 接口与参数（与 PR 字段对齐）
sink ExportAuthInfo 入参：

| 字段 | 值 |
|---|---|
| `pinConsumerPkgName` | `CollaborationFwk` |
| `pinConsumerTokenId` | `EnsureCollaborationFwkToken().selfAfter`（即 SetSelfTokenID 后的 native token id） |
| `bizSrcPkgName` / `bizSinkPkgName` / `regPkgName` | `CollaborationFwk` |
| `userId` | 当前前台用户 ID（MultipleUserConnector） |
| `authType` | `TRUST_ALWAYS(6)` |
| `authBoxType` | `SKIP_CONFIRM(2)` |
| `pinExchangeType` | `IMPORT_AUTH_CODE(5)` |
| `extraInfo` | `{"ACL_LIFE_CYCLE_DAYS":1095}` |
| `pinLength` | 6 |

sink ImportAuthInfo 入参：复用 sink ExportAuthInfo 出参，`pinCode` 字段用 DM 写回的 6 位 PIN。

source ImportAuthInfo 入参：复用 sink 同字段，`pinCode` 用 xdevice 传过来的 6 位 PIN。

source BindTarget bindParam：`{AUTH_TYPE:"6", BIND_LEVEL:"1", BUNDLE_NAME:"CollaborationFwk"}`（PIN 已由 ImportAuthInfo 注册到 source 的 authCodeMap）。

### PC 端 ACL 验证细节（`xdevice/lib/acl_db_verifier.py`）

1. 远端目录：`/data/service/el1/public/database/distributed_device_profile_service/acl_db`，包含 `*.db` 主文件以及 SQLite WAL 模式产生的 `*.db-shm` / `*.db-wal`
2. 用 `hdc file recv -r` 整目录拉到 PC 临时目录（WAL 必须与主 db 同时拉过来，否则 SQLite 读不到最新写入）
3. 用 Python 内置 `sqlite3` 以只读 URI 模式打开每个 `*.db`
4. 通过 `sqlite_master` 列出所有 table，再 `PRAGMA table_info` 动态探测 ACL 表与 `AccessControlProfile.extraData` 对应列——表名和列名是 DP 实现细节，不写死
5. 在该 ACL 表上按包名或关系字段定位目标 ACL 行，读取 `AccessControlProfile.extraData` 对应列
6. `AccessControlProfile.extraData` 是 JSON 字符串列；`json.loads` 后断言 `obj["ACL_LIFE_CYCLE_DAYS"] == 1095` 且为 int 类型（非 bool）
7. DP 写库异步，bind 完成 ≠ ACL 立即可见，**轮询拉**：`poll_max_ms=15000`，`poll_interval_ms=500`；命中 `row_found && !value_matches` 直接返回失败（不再重试）

### 断言矩阵
| ID | 断言 | 落点 | 失败 reason |
|---|---|---|---|
| A1 | sink ExportAuthInfo 返回 0 且 pin 长度==6 全数字 | sink stdout | EXPORT_AUTHINFO |
| A2 | PIN 文件成功 ferry 到 source | run_test.py | PIN_FERRY |
| A3 | source ImportAuthInfo 返回 0 | source stdout | IMPORT_AUTHINFO |
| A4 | source 30s 内 OnDeviceFound 拿到非空 wifiIp | source stdout | DISCOVER_TIMEOUT |
| A5 | source OnBindResult.result == 0 | source stdout | BIND_FAIL_\<errCode\> |
| A6 | PC 端拉 RDB 后能查到 PKG 对应 ACL 行 | acl_db_verifier | ACL_NOT_PERSISTED / HDC_RECV_FAIL / DB_NO_FILE |
| A7 | `AccessControlProfile.extraData` JSON 含 key `ACL_LIFE_CYCLE_DAYS` | acl_db_verifier | FIELD_MISSING / EXTRA_DATA_NOT_JSON |
| A8 | 该 key 值 == 1095（int） | acl_db_verifier | VALUE_MISMATCH / VALUE_NOT_INT |

A1..A8 全通过 → case PASS。

### 报告产物
路径：`/home/luna/workspace/issue_2445_auto_test_report/`，包含 `summary.md`、`results/source_result.json`、`results/sink_result.json`（PC 端验证结果合成）、两端 hilog（已掩码 udid/networkId）。

### 约束与风险
- 二进制名 `CollaborationFwk_sink` / `CollaborationFwk_source` 仅是磁盘 / `argv[0]` 标识；DM 的 `AUTH_CODE_WHITE_LIST` 是**精确等值**比较，靠 `EnsureCollaborationFwkToken()` 在 `main()` 起始覆写 self token 后通过校验。覆写失败 demo 立刻退出并打 `self_token_setup_failed:<reason>`
- 当宿主 OH 版本调整 `access_token` 权限模型时（例如 shell 用户不再允许调 `GetAccessTokenId`，或同名 native token 不再覆盖而冲突），用归档仓 `p2_probe` 直接复测，30 秒内判定是否需要回退到系统 HAP 方案
- 开源镜像下 `ExportAuthInfo` / `ImportAuthInfo` IPC 入口应返回 `DM_OK`；`ERR_DM_UNSUPPORTED_METHOD (96929776)` 仅用于闭源/商用 resident so 缺失或加载异常等场景（见 D-011）。ACL `ACL_LIFE_CYCLE_DAYS` 实际写入发生在 **sink 状态机 bind 阶段** `AuthSinkConfirmState::Action`（开源 `auth_confirm.cpp`）之后的 ACL 写入流程，与 resident so 无关
- sink demo **不**链接 DP SDK，也不在板上读 sqlite——ACL 验证全部下放到 PC 端 Python 脚本，避免设备侧权限/数据库锁问题
- DP RDB 表名/列名为 DP 实现细节，`acl_db_verifier.py` 通过 `PRAGMA table_info` 动态发现 ACL 表与 `AccessControlProfile.extraData` 对应列；如未来 DP 改列命名风格（如 snake_case），在 ACL extraData 列候选中补一行即可
- `hdc file recv` `/data/service/el1/public/...` 目录在常规 RK3568 用户版上默认可读；如失败 PC 端会以 `HDC_RECV_FAIL:<stderr>` 显式报错而不是默默通过
- PIN 单次有效：失败重跑必须重新 `ExportAuthInfo` + `ImportAuthInfo`
- demo 源码归档在独立测试仓，PR 行为代码不携带 testcode；不破坏 ABI/IPC

### 知识库引用
- `docs/04-workflows/08-import-pin-no-popup-bind.md`：新接口（Import/ExportAuthInfo）的完整调用流程与陷阱
- `foundation/deviceprofile/device_info_manager/permission/permission.json`：DP API 进程名白名单（为什么 sink demo 不能直接调 DP SDK）
