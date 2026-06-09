# 免弹框 PIN 码绑定流程（Export + Import AuthInfo）

> 适用场景：自动化测试 / 安全场景下需要跳过 PIN 弹框 UI 的绑定。
> 关键点：弹框为 SecComp 安全页面，外部进程无法截图识别，必须走 `ImportAuthInfo` 路径。
> 本文档同时覆盖**旧 `ImportAuthCode`** 与**新 `ImportAuthInfo` / `ExportAuthInfo`**（详见 §2）。

---

## 1. 涉及的接口

来源：`interfaces/inner_kits/native_cpp/include/device_manager.h`

| 接口 | 说明 |
|---|---|
| `int32_t ExportAuthInfo(DmAuthInfo &dmAuthInfo, uint32_t pinLength)` | **新接口（推荐）**。DM 服务端生成 `pinLength` 位（6~1024）随机 PIN 写回 `dmAuthInfo.pinCode`，并把调用方填好的 `pinConsumerPkgName/authType/pinExchangeType/authBoxType/userId/extraInfo` 暂存到 `tokenIdPinCodeMap_`。同时启动 PIN 失效定时器 `BIND_TARGET_PIN_TIMEOUT_TASK` |
| `int32_t ImportAuthInfo(const DmAuthInfo &dmAuthInfo)` | **新接口（推荐）**。校验 `IsImportAuthInfoValid`（含 `IsAclLifeCycleDaysValid`），通过后调 `PutLocalServiceInfo` 写 DP 并内部 `ImportAuthCode(pkgName, pin)`；source 与 sink **都调这个**，sink 调一次完成"PIN 注册 + LocalServiceInfo 写入"两件事，不必再额外调 `RegisterLocalServiceInfo` |
| `int32_t ExportAuthCode(std::string &authCode)` | **旧接口**。仅生成全局随机 6 位 PIN，不携带元数据 |
| `int32_t ImportAuthCode(const std::string &pkgName, const std::string &authCode)` | **旧接口**。把外部传入的 PIN 码绑定到 `pkgName` 上；多次调用只保留最后一次 |
| `int32_t RegisterLocalServiceInfo(const DMLocalServiceInfo &info)` | sink 侧手工注册本地服务信息；与旧 `ImportAuthCode` 搭配使用。**走新接口可省略** |
| `int32_t BindTarget(pkgName, PeerTargetId, bindParam, callback)` | source 侧发起绑定。`bindParam` 含 `AUTH_TYPE`、`BIND_LEVEL` 等 |

## 2. 新旧接口对比

| 维度 | 旧 `Export/ImportAuthCode` | 新 `Export/ImportAuthInfo` |
|---|---|---|
| 入参 | 字符串 `pkgName` + 字符串 `authCode` | 结构体 `DmAuthInfo`（含 userId/tokenId/authType/pinExchangeType/authBoxType/extraInfo/...） |
| PIN 长度 | 固定 6 位 | `pinLength` 参数，范围 `[DM_MIN_PINCODE_SIZE=6, DM_MAX_PINCODE_SIZE=1024]` |
| 必须配套 `RegisterLocalServiceInfo` | 是 | **否**——`ImportAuthInfo` 内部已 `PutLocalServiceInfo` |
| 承载 `ACL_LIFE_CYCLE_DAYS` | 不能，需走 `RegisterLocalServiceInfo.extraInfo` | **直接**通过 `DmAuthInfo.extraInfo` JSON 字段携带 |
| 服务端校验 | 仅长度/数字 | `IsExportAuthInfoValid` + `IsImportAuthInfoValid` + `IsAclLifeCycleDaysValid`（authType=TRUST_ALWAYS 才允许带 `ACL_LIFE_CYCLE_DAYS`） |
| 调用方 process name 白名单 | 同样要求 | 同样要求（见 §3） |
| PIN 自动失效 | 由后续 bind 消耗 | `StartAuthInfoTimer` 到期会调 `DeleteLocalServiceInfo` + 通知 `OnAuthCodeInvalid`（见 `device_manager_service_impl.cpp:3634`） |
| 适用 | 老协议路径 / 兼容 | **推荐**所有新自动化测试与新业务 |

## 3. 调用方进程名白名单（重要！）

`ImportAuthInfo` 与 `ExportAuthInfo` 都调用 `PermissionManager::CheckProcessNameValidOnAuthCode(processName)`——拿的是 **调用进程的实际 process name**（IPC 元信息），不是 pkgName。白名单（`services/service/src/permission/standard/permission_manager.cpp:36`）：

```
CollaborationFwk
wear_link_service
watch_system_service
cast_engine_service
glasses_collaboration_service
xr_glass_app_service
gameservice_server
caas_service
```

二进制名字本身不重要——DM 拿的是 IPC 元信息里的 **self token processName**，
与 `argv[0]` 无关。白名单是**精确等值**匹配，**非前缀**。

调用方有两种合法路径让自己的 `processName` 命中白名单：

- **真正注册为系统 native SA**：在 `sa_profile/*.cfg` / nativetoken 表里注册一个名为 `CollaborationFwk`（或其它白名单项）的服务，进程启动即生效。这是生产代码的标准做法。
- **测试 demo 通过 `SetSelfTokenID` 覆写 self token**：在 `main()` 起始调用 `GetAccessTokenId({.processName="CollaborationFwk", .aplStr="system_core", .perms=...}) + SetSelfTokenID + AccessTokenKit::ReloadNativeTokenInfo()` 三件套——DM UT `test/unittest/UTTest_dm_pin_holder.cpp` 的 `SetUp()` 即是此手段。issue-2445 自动化测试也走这条路（封装在独立测试归档仓 `issue-2445-acl-life-cycle-days/common/sec_token_kit.cpp`），二进制名只是磁盘标识；归档仓 `p2_probe` 已在 RK3568 上验证该手段对 shell 用户可行。

否则 `ExportAuthInfo` / `ImportAuthInfo` 直接 `ERR_DM_NO_PERMISSION`。

注：`pinConsumerPkgName` 字段允许是任意合法字符串（仅校验非空 + 长度 ≤ `DM_PARAM_STRING_LENGTH_MAX`），用作 LocalServiceInfo 的主键，与 process name 解耦。

## 4. PIN 码取值范围

- **新接口**：长度 `[DM_MIN_PINCODE_SIZE=6, DM_MAX_PINCODE_SIZE=1024]`；`ExportAuthInfo` 生成的 PIN 为全数字，`ImportAuthInfo` 入口只校验长度
- **旧接口**：固定 6 位（`100000~999999`）
- 选 6 位即与新旧两端的最小公倍数兼容

## 4.5. IPC 第二段单/双兼容适配 so（开源镜像端到端必读）

`DeviceManagerService::ExportAuthInfo` / `ImportAuthInfo` IPC 入口**两段式**：

1. **开源第一段**：`DeviceManagerServiceImpl::ExportAuthInfo` / `ImportAuthInfo`，纯开源
   - `ExportAuthInfo` (line 3610)：生成 PIN，写回 `dmAuthInfo.pinCode`，存内存 `tokenIdPinCodeMap_`，启 PIN timer
   - `ImportAuthInfo` (line 3518)：`PutLocalServiceInfo(extraInfo含ACL_LIFE_CYCLE_DAYS)` 落 sink/source 的 DP RDB，`ImportAuthCode(pkgName, pinCode)` 把 PIN 注册到 DM 的 auth code map
2. **闭源第二段**：`dmServiceImplExtResident_->ExportAuthInfo` / `ImportAuthInfo`，单/双设备兼容适配层
   - 闭源版本通过 `dlopen("libdevicemanagerresident.z.so", RTLD_NOW | RTLD_NODELETE)` 动态加载（`device_manager_service.cpp:1866-1871`、`:1823-1828`）
   - 这个 so **不在 OH 开源仓**，未开源；OpenHarmony community RK3568 镜像不带

**开源镜像表现**：

- `DEVICE_MANAGER_COMMON_FLAG` 定义时，IPC 入口跳过闭源第二段并直接返回 `DM_OK`
- 这样 `ExportAuthInfo` 的 IPC stub 才会 encode/decode `DmAuthInfo`，把 `pinCode` 等字段正常回传给 SDK 调用方；`ImportAuthInfo` 只回传错误码
- ACL `ACL_LIFE_CYCLE_DAYS` 实际写入发生在**后续 bind 阶段** sink 侧 `AuthSinkConfirmState::Action`（开源 `services/implementation/src/authentication_v2/auth_stages/auth_confirm.cpp`），完全不依赖第二段 resident so

**结论**：在 OpenHarmony 开源镜像上，`ExportAuthInfo`/`ImportAuthInfo` 应返回 `DM_OK` 并继续后续 `BindTarget`：

- 开源段副作用已完成（PIN 已在 DM 内存暂存、LocalServiceInfo 已在 DP 持久化、PIN 已在 auth code map 注册）
- 后续 bind 流程能正常触发 sink 状态机读 LocalServiceInfo.extraInfo 并把 `ACL_LIFE_CYCLE_DAYS` 落到 `AccessControlProfile.extraData`
- `ERR_DM_UNSUPPORTED_METHOD (96929776)` 只应出现在闭源/商用版本 resident so 缺失或加载异常等场景
- A1-A8 端到端断言矩阵在**开源 OH 镜像上完全可跑通**（不再需要带闭源 resident so 的商用镜像）

**判定开源段是否真正成功**：

```bash
# 看 hilog 是否出现开源段日志（关键字 dmServiceImpl）
hdc shell "hilog -x | grep -E 'DeviceManagerServiceImpl.*Import|PutLocalServiceInfo'"

# 看 DP local_service_info_store.db 是否新增了 row
hdc file recv /data/service/el1/public/database/distributed_device_profile_service/local_service_info_store.db /tmp/lsi.db
sqlite3 /tmp/lsi.db "SELECT name FROM sqlite_master WHERE type='table'"
sqlite3 /tmp/lsi.db "SELECT bundleName, extraInfo FROM local_service_info_store"
```

如果开源段已成功，`local_service_info_store.db` 应能查到 sink/source 写入的 row，且 `extraInfo` 列含 `ACL_LIFE_CYCLE_DAYS`。

**业务侧 fallback 矩阵**：

| 测试目的 | 是否被开源镜像 resident so 缺失阻塞 | 替代验证手段 |
|---|---|---|
| 验证 `IsAclLifeCycleDaysValid` / `IsExportAuthInfoValid` 等入口校验 | 否（第一段开源代码内） | DM UT（`UTTest_device_manager_service*`） |
| 验证 `auth_confirm.cpp::ProcessImportAuthInfo` 解析 `ACL_LIFE_CYCLE_DAYS` 并写入 `AccessControlProfile.extraData` | 否（bind 阶段开源代码内） | DM UT；端到端可跑（开源镜像） |
| 验证 `device_manager_service_impl.cpp::DeleteAlwaysAllowTimeOut` 周期清理使用 per-ACL 的 lifecycle days | 否 | DM UT |
| 端到端 `ExportAuthInfo → ferry PIN → ImportAuthInfo → BindTarget → 查 ACL` | **否**（开源版返回 `DM_OK`） | A1-A8 在开源 RK3568 镜像可全跑通 |


## 5. 免弹框 bind 必跑步骤（新接口路径，推荐）

sink 侧（self token `processName=CollaborationFwk`，见 §3 说明）：

1. 填 `DmAuthInfo`：`pinConsumerPkgName="CollaborationFwk"`、`pinConsumerTokenId=<EnsureCollaborationFwkToken().selfAfter>`、`bizSrcPkgName/bizSinkPkgName/regPkgName="CollaborationFwk"`、`userId=<前台用户>`、`authType=TRUST_ALWAYS(6)`、`authBoxType=SKIP_CONFIRM(2)`、`pinExchangeType=IMPORT_AUTH_CODE(5)`、`extraInfo` 含 `ACL_LIFE_CYCLE_DAYS`
2. 调 `ExportAuthInfo(authInfo, 6)`，DM 写回 `authInfo.pinCode`，开源镜像应返回 `DM_OK` 并通过 IPC 回传 PIN（见 §4.5）
3. 复用 `authInfo` 再调一次 `ImportAuthInfo(authInfo)`（同 PIN + 同 extraInfo），让 sink 本机 DP 持久化 LocalServiceInfo，开源镜像应返回 `DM_OK`
4. 把 pin、PKG、authInfo 字段通过 xdevice 带外传给 source

source 侧（self token `processName=CollaborationFwk`，见 §3 说明）：

5. 填 `DmAuthInfo`（与 sink 同 `pinConsumerPkgName / pinExchangeType / authType / authBoxType`，`pinCode` 用收到的 pin）
6. 调 `ImportAuthInfo(authInfo)`，DM 校验 + `PutLocalServiceInfo` + 内部 `ImportAuthCode(PKG, pin)`，开源镜像应返回 `DM_OK`
7. `StartDiscovering`，等 `OnDeviceFound` 拿 `wifiIp / wifiPort`
8. 调 `BindTarget(PKG, PeerTargetId{wifiIp,wifiPort}, bindParam={AUTH_TYPE=6, BIND_LEVEL=DEVICE, BUNDLE_NAME="CollaborationFwk"})`；PIN 由 step 6 暂存复用
9. 等 `OnBindResult.result == 0`

关键不变式：

- self token 的 `processName` 必须命中 §3 白名单；二进制 demo 通过 `SetSelfTokenID` 把 `processName` 覆写为 `CollaborationFwk`（DM UT `UTTest_dm_pin_holder` 同款手段）
- `Init(pkgName)` 与 `DmAuthInfo.pinConsumerPkgName` / `bizSrcPkgName` / `bizSinkPkgName` 必须**等于** self token 的 processName `"CollaborationFwk"`：既满足 `AUTH_CODE_WHITE_LIST` 精确等值，又让 `AuthSinkConfirmState` 走 `BUNDLE_NAME_COLLABORATION_FWK` 兼容分支
- sink 与 source 的 `pinConsumerPkgName` 必须完全一致（DP 查 PIN 的主键）
- sink 与 source 的 `pinExchangeType` 必须一致
- `extraInfo` 带 `ACL_LIFE_CYCLE_DAYS` 时 `authType` 必须是 `TRUST_ALWAYS(6)`，否则 `IsAclLifeCycleDaysValid` 拒绝
- PIN 单次有效：bind 完成或 `BIND_TARGET_PIN_TIMEOUT` 到期后失效，重跑必须重 Export+Import
- 开源镜像下 `ExportAuthInfo` / `ImportAuthInfo` 应返回 `DM_OK`；`ERR_DM_UNSUPPORTED_METHOD (96929776)` 仅用于闭源/商用 resident so 缺失或加载异常等场景

## 6. 为什么用 ImportAuthInfo 而不用 PIN 弹框

- PIN 弹框由 `display/entry` HAP 实现，是 SecComp 安全页面
- 安全页面禁止外部进程截图、读取像素、注入输入事件
- 自动化测试无法 OCR 识别 PIN，也无法 mock 输入，必须走 ImportAuthInfo 路径

## 7. 常见陷阱

| 现象 | 根因 |
|---|---|
| `Export/ImportAuthInfo` 返回 `ERR_DM_NO_PERMISSION` | 进程名不在 §3 白名单 |
| `ImportAuthInfo` 返回 `ERR_DM_INPUT_PARA_INVALID` | `userId` 不在前台用户列表；或 `pinCode` 长度非 `[6,1024]`；或 `authType` 不属于 `{0,6}`；或 `authBoxType / pinExchangeType` 越界；或 `extraInfo` JSON Discarded；或 `ACL_LIFE_CYCLE_DAYS` 与 `authType` 不匹配 |
| bind 成功但 ACL 无 `ACL_LIFE_CYCLE_DAYS` | `authType` 不等于 `TRUST_ALWAYS(6)` 被前置剔除（PR commit `5b1f68ac7`） |
| bind 弹出 PIN 框 | sink 与 source 的 `pinConsumerPkgName` 或 `pinExchangeType` 不一致；或 source 未调 `ImportAuthInfo` |
| `ExportAuthInfo` 后过几秒再 bind 报 PIN 失效 | `BIND_TARGET_PIN_TIMEOUT` 到期触发 `DeleteLocalServiceInfo` |
| 多次 `ImportAuthInfo` 用了旧 PIN | DM 只保留最后一次 `PutLocalServiceInfo` 与 `tokenIdPinCodeMap_` 条目 |
| 二进制 demo 进程名是自起名字 | 不在白名单，直接 `ERR_DM_NO_PERMISSION`；必须落盘名取自白名单 |

## 8. ACL_LIFE_CYCLE_DAYS 与本流程的关系

- 字段：`common/include/dm_constants.h::ACL_LIFE_CYCLE_DAYS = "ACL_LIFE_CYCLE_DAYS"`
- 范围：`[ACL_LIFE_CYCLE_DAYS_MIN=1, ACL_LIFE_CYCLE_DAYS_MAX=3650]`，未配置 `= -1`
- 新接口写入路径：sink `ExportAuthInfo` 的 `DmAuthInfo.extraInfo` 含 `ACL_LIFE_CYCLE_DAYS`；`InitDpServiceInfo` 把整段 extraInfo 写入 DP LocalServiceInfo；bind 成功后透传到 `DmAuthContext` 再持久化到 `AccessControlProfile.extraData`
- 仅当 `authType == TRUST_ALWAYS(6)` 时该字段才允许并最终落盘
- 数据库路径：`/data/service/el1/public/database/distributed_device_profile_service/acl_db/`；ACL 关系级字段在 `AccessControlProfile.extraData` 对应列中，实体补充信息才在 `accesser/accessee` extraData

## 9. xdevice 自动化的最小副作用断言点（新接口）

1. sink `ExportAuthInfo` 返回 DM_OK 且 `authInfo.pinCode` 长度等于入参 `pinLength` 且全数字
2. source `ImportAuthInfo` 返回 DM_OK
3. source `OnBindResult.result == 0`
4. **PC 端**（xdevice 脚本，不在板上跑）通过 `hdc file recv -r /data/service/el1/public/database/distributed_device_profile_service/acl_db` 把 sink 板上的 DP RDB 整目录拉到本机临时目录，再用 Python 内置 `sqlite3` 打开 `*.db` 主文件，动态定位 ACL 表中的 `AccessControlProfile.extraData` 对应列，校验该 JSON 列解析出 `ACL_LIFE_CYCLE_DAYS` 整数 == sink 注册值

> 注意：**不要**在测试 demo 里直接调用 `DistributedDeviceProfileClient::GetAllAccessControlProfile`。该接口在 DP 的 `foundation/deviceprofile/device_info_manager/permission/permission.json` 中受调用进程名白名单约束，只放行 `device_manager` 和 `softbus_server`；自动化测试 demo 即使把 self token 覆写为 `CollaborationFwk`（DM 白名单内）也不在 DP 白名单内，调用必返回失败。改为 PC 端拉 RDB 本地解析是绕开这一限制的标准做法，同时也避免了在板上链接 DP SDK / 处理 sqlite 锁的额外复杂度。完整的 DP API 白名单清单与板上各类 RDB 文件位置见 [DP 权限白名单与板上数据库位置](../05-interfaces/04-dp-permission-and-storage.md)。

---
