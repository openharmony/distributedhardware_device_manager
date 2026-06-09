# DP 接口调用方权限白名单与板上数据存储位置

版本 v1.0
更新日期 2026-05-30

## 1. 适用场景

任何 DM 代码、调试工具、自动化测试用例在跨进程调用 `DistributedDeviceProfileClient`（DP SDK）前，**都必须先确认调用方进程名是否在 DP 的权限白名单内**。白名单未命中的进程调用会被 DP 服务直接拒绝（typical 错误码 `DP_INVALID_ACCESS` / `ERR_DM_NO_PERMISSION`，依调用方框架而定），DM 侧通常表现为返回非零或空列表。

同样地，**自动化测试需要校验 ACL/SessionKey/LocalServiceInfo 的实际持久化结果**时（例如验证 PR 写入了新字段），如果调用方不在白名单里，正确的做法是**从板上拉数据库文件**，再用本机工具解析，而不是绕开权限校验。本文档把这两件事的事实依据集中在一起。

## 2. DP API 调用方进程名白名单

**唯一权威来源**：`foundation/deviceprofile/device_info_manager/permission/permission.json`

> DP 服务端在每次接受 IPC 调用时读取该 JSON，按 caller 的实际 process name（IPC 元信息里的 `processName`，**不是** `bundleName`、**不是** `tokenId`）做精确字符串匹配。命中则放行，未命中即拒绝。

### 2.1 当前白名单（截至 2026-05-30）

| API 类别 | 接口 | 允许的进程名 |
|---|---|---|
| ACL 写 | `PutAccessControlProfile` / `UpdateAccessControlProfile` / `DeleteAccessControlProfile` | `device_manager`, `softbus_server` |
| ACL 读 | `GetAccessControlProfile` | `device_manager`, `softbus_server`, `iShare` |
| ACL 全量读 | `GetAllAccessControlProfile` / `GetAllAclIncludeLnnAcl` / `GetTrustDeviceProfile` / `GetAllTrustDeviceProfile` | `device_manager`, `softbus_server` |
| 信任设备批量写 | `PutAllTrustedDevices` | `device_manager` |
| ServiceInfoProfile（全部） | `PutServiceInfoProfile` / `DeleteServiceInfoProfile` / `UpdateServiceInfoProfile` / `GetServiceInfoProfileByUniqueKey` / `GetServiceInfoProfileListByTokenId` / `GetAllServiceInfoProfileList` / `GetServiceInfoProfileListByBundleName` | `device_manager` |
| ServiceInfo（全部） | `PutServiceInfo` / `DeleteServiceInfo` / `GetAllServiceInfoList` / `GetServiceInfosByUserInfo` | `device_manager` |
| LocalServiceInfo（全部） | `PutLocalServiceInfo` / `UpdateLocalServiceInfo` / `GetLocalServiceInfoByBundleAndPinType` / `DeleteLocalServiceInfo` | `device_manager` |
| 设备/服务/特征 Profile | `PutServiceProfile(Batch)` / `PutCharacteristicProfile(Batch)` / `GetDeviceProfile` / `GetServiceProfile` / `GetCharacteristicProfile` / `DeleteServiceProfile` / `DeleteCharacteristicProfile` / `SubscribeDeviceProfile` / `UnSubscribeDeviceProfile` / `SyncDeviceProfile` | `all`（无白名单约束） |
| DeviceProfile 批量 | `PutDeviceProfileBatch` / `DeleteDeviceProfileBatch` / `GetDeviceProfiles` / `PutProductInfoBatch` / `PutDeviceIconInfoBatch` / `GetDeviceIconInfos` | `device_manager` |
| SessionKey | `PutSessionKey` / `GetSessionKey` / `UpdateSessionKey` / `DeleteSessionKey` | `device_manager`, `softbus_server` |

> 更新提醒：DP 仓为权威源；本表为 DM 侧快照。若 PR 改动 DP `permission.json`，请同步更新本节。

### 2.2 实操含义

- DM 自己的 `services/service` 进程注册 SA 名 `device_manager`，所有 DP 调用天然合法
- DM 工具/Demo（无论叫什么 bundleName）只要进程名不是 `device_manager` / `softbus_server`，**绝大多数 DP 接口都调不通**
- 自动化测试 demo（如 issue-2445 的 `CollaborationFwk_sink`）即便通过 `SetSelfTokenID` 把 self token 的 `processName` 覆写为 `CollaborationFwk`（DM 白名单内），也**不在**这里的 DP 白名单 `device_manager` / `softbus_server` 内，仍**不能**直接调用 `GetAllAccessControlProfile` 验证 ACL 写入结果——会被 DP 拒绝
- 命中 `iShare` 的接口只放行单一进程，DM 自动化测试同样调不通

### 2.3 替代方案：自动化测试如何校验 ACL 写入

参考独立测试归档仓 `issue-2445-acl-life-cycle-days/xdevice/lib/acl_db_verifier.py` 的实践：

1. 测试 demo 触发完业务流程后，**PC 端 xdevice 脚本**用 `hdc file recv -r <DB 目录>` 把 DP RDB 整目录拉到本机临时目录
2. 用本机 Python 内置 `sqlite3` 模块（或 sqlite3 CLI）以只读模式打开 `*.db` 主文件
3. 通过 `PRAGMA table_info` 动态发现含 `accesseeBundleName` / `accesseeExtraData` 列的 ACL 表（DP 表名为实现细节，不写死）
4. 在该表上按业务 PKG 过滤，对目标 JSON 字段做断言

> WAL 注意：DP 默认走 SQLite WAL 模式，`*.db-shm` / `*.db-wal` 必须与主 `.db` 一起拉到 PC，否则 SQLite 引擎读不到最新写入

## 3. DP 持久化文件在板上的位置

DP 在 OpenHarmony 设备上把所有持久化数据放在 `el1` 公共加密分区下，目录结构如下（截至 2026-05-30，按 distributed_device_profile 当前实现）：

| 数据类别 | 板上路径 | 文件 | 说明 |
|---|---|---|---|
| ACL（AccessControlProfile） | `/data/service/el1/public/database/distributed_device_profile_service/acl_db/` | `*.db` + WAL 旁路文件 (`*.db-shm`, `*.db-wal`) | 含 `accesseeBundleName`、`accesseeExtraData`、`accesserBundleName`、`accesserExtraData`、`bindType`、`bindLevel`、`authenticationType` 等字段；DM ACL 全生命周期数据落点 |
| TrustDevice / TrustedDevice | `/data/service/el1/public/database/distributed_device_profile_service/` 下另起 RDB | 同上模式 | 由 `PutAllTrustedDevices` / `GetAllTrustDeviceProfile` 维护 |
| ServiceInfoProfile / ServiceInfo / LocalServiceInfo | `/data/service/el1/public/database/distributed_device_profile_service/` 下另起 RDB | 同上模式 | DM `RegisterLocalServiceInfo` / `PutLocalServiceInfo` 落点；issue-2445 中 sink 端写入 `extraInfo` 后先落在此处 |
| SessionKey | `/data/service/el1/public/database/distributed_device_profile_service/` 下另起 RDB（带加密属性，文件级权限更严） | 同上模式 | DM 与 SoftBus 共用 |
| DeviceProfile / ServiceProfile / CharacteristicProfile | `/data/service/el1/public/database/distributed_device_profile_service/` | KV / RDB 混合 | 通用 profile 信息 |

> 同一进程的多个 RDB 文件由 DP 服务初始化时按 schema 分别建库，子目录命名规则随 DP 版本演进；**自动化测试代码不要硬编码表名/库文件名**，而是用 `PRAGMA table_list` / `PRAGMA table_info` 在 PC 端动态发现。

### 3.1 板上访问注意事项

- 路径在 `el1`，需在前台用户解锁后才可读写；PC 端 `hdc file recv` 默认可读用户版镜像的该目录
- `hdc file recv -r` 必须递归拉整目录，避免漏掉 WAL 旁路文件
- 不要在板上直接 `sqlite3 dp_rdb.db` 后 `INSERT`/`UPDATE`——DP 服务运行中会持锁写入，外部写入可能破坏 DB；**只读拉到 PC 上分析**

## 4. 关联文档

- `foundation/deviceprofile/device_info_manager/permission/permission.json`：权威进程名白名单（DP 仓）
- `docs/05-interfaces/03-dependency-api.md` §4：DM 实际调用 DP 的接口清单
- `docs/04-workflows/08-import-pin-no-popup-bind.md` §9：免弹框 PIN 流程下的 ACL 验证最小副作用断言点（含 PC 端拉 RDB 的具体步骤）
- 独立测试归档仓 `issue-2445-acl-life-cycle-days/xdevice/lib/acl_db_verifier.py`：PC 端拉库+解析+轮询的参考实现
- `.codespec/changes/issue-2445-acl-life-cycle-days/design.md`：分布式自动化测试设计（说明为什么 sink demo 不直接调 DP SDK）
