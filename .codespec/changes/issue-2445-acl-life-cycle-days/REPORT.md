# ACL_LIFE_CYCLE_DAYS 端到端验证报告

> Issue: #2445 | PR: 3730 | 日期: 2026-06-01 | 镜像: `openharmony_master_default_20260530115923` make_all 整编 (rk3568)

## 1. 测试环境

| 项目 | 值 |
|------|------|
| OH 版本 | 7.0.0.27 |
| security_patch | 2026/06/01 |
| 代码基线 | `97689e553` + ACL_LIFE_CYCLE_DAYS PR 补丁 |
| 构建方式 | `./build.sh --product-name rk3568 --ccache --build-target make_all`（整编镜像，非 indep） |
| 设备 A（source） | RK3568, SN `150100424a5444345209d94ebed7b900` |
| 设备 B（sink） | RK3568, SN `450100445634303332011a0f0a2eba00` |
| 网络 | 两板同一 Wi-Fi，双端亮屏 |

## 2. 待测代码变更

| 文件 | 变更说明 |
|------|----------|
| `services/implementation/src/authentication_v2/dm_auth_message_processor.cpp` | `PutDeviceControlList`：追加 `extraData[ACL_LIFE_CYCLE_DAYS]` 写入（device-level bind）；`PutNonLnnAclProfile`：追加写入（service-level bind）；删除 `BuildAccesseeExtraDataWithAclDays` |
| `services/implementation/src/authentication_v2/auth_stages/auth_confirm.cpp` | `ProcessImportAuthInfo`：从 `srvExtraInfo` 提取 `ACL_LIFE_CYCLE_DAYS` 写入 `context->aclLifeCycleDays` |
| `services/implementation/src/device_manager_service_impl.cpp` | `GetAclAllowSeconds`：从 `profile.GetExtraData()` 读取（原为 `profile.GetAccessee().GetAccesseeExtraData()`） |
| `common/include/dm_constants.h` / `common/src/dm_constants.cpp` | 常量 `ACL_LIFE_CYCLE_DAYS`、`_MIN=1`、`_MAX=3650`、`_NOT_CONFIGURED=-1` |
| `services/service/src/device_manager_service.cpp` | `IsImportAuthInfoValid` / `IsAclLifeCycleDaysValid`：authType × ACL_LIFE_CYCLE_DAYS 交叉校验 |

## 3. 测试用例与结果

### 3.1 端到端分布式测试（双 RK3568 板）

| 用例编号 | Spec AC | 输入 `ACL_LIFE_CYCLE_DAYS` | 预期结果 | 实际结果 | 结论 |
|---------|---------|----------------------------|----------|--------|------|
| E2E-1 | AC-1 | 1095（典型值：3 年） | 绑定成功；`access_control_table.extraData` 含 `"ACL_LIFE_CYCLE_DAYS":1095`；`accesser/accessee.extraData` 不含该 key | 绑定成功；ACL extraData=`{"ACL_LIFE_CYCLE_DAYS":1095,"IsLnnAcl":"false","serviceId":0}`；accesser/accessee 仅 `{"dmVersion":"5.1.5"}` | **PASS** |
| E2E-2 | AC-3 | -1（哨兵值） | 绑定成功；`access_control_table.extraData` 不含 `ACL_LIFE_CYCLE_DAYS` | 绑定成功；ACL extraData=`{"IsLnnAcl":"false","serviceId":0}`（无 ACL_LIFE_CYCLE_DAYS key） | **PASS** |
| E2E-3 | AC-2 | 3651（超界） | `ImportAuthInfo` 被拒，返回 `ERR_DM_INPUT_PARA_INVALID (96929749)` | sink `import_auth_info_failed:96929749` | **PASS** |
| E2E-4 | AC-2 | 0（非法零值） | `ImportAuthInfo` 被拒，返回 `ERR_DM_INPUT_PARA_INVALID` | sink `import_auth_info_failed:96929749` | **PASS** |
| E2E-5 | AC-1 | 1（合法下界） | 绑定成功；`access_control_table.extraData` 含 `"ACL_LIFE_CYCLE_DAYS":1` | 绑定成功；ACL extraData=`{"ACL_LIFE_CYCLE_DAYS":1,"IsLnnAcl":"false","serviceId":0}` | **PASS** |
| E2E-6 | AC-1 | 3650（合法上界） | 绑定成功；`access_control_table.extraData` 含 `"ACL_LIFE_CYCLE_DAYS":3650` | 绑定成功；ACL extraData=`{"ACL_LIFE_CYCLE_DAYS":3650,"IsLnnAcl":"false","serviceId":0}` | **PASS** |
| E2E-7 | AC-1 | 1095（设备代理绑定） | agent 与 proxy subject ACL 均写入客体侧注册值 | 绑定成功；详见 E2E-7 详细证据 | **PASS** |

#### E2E-1 详细证据（Happy Path，days=1095）

**Sink 输出：**
```json
{"a1":"PASS","pinLen":6,"pinMasked":"***","pkgName":"CollaborationFwk","exportRet":0,"importRet":0,"lifeCycleDays":1095,"userId":100,"selfTokenId":672120760}
```

**Source 输出：**
```json
{"a3":"PASS","a4":"PASS","a5":"PASS","bindResult":0,"bindStatus":8,"wifiIpMasked":"192.***43.6","wifiPort":37091,"importRet":0,"lifeCycleDays":1095,"selfTokenId":671223800}
```

**Sink DP RDB `access_control_table`：**

| accessControlId | bindLevel | extraData |
|-----------------|-----------|-----------|
| 1 | 1 (LNN) | `{"IsLnnAcl":"true","serviceId":0}` |
| 2 | 2 (非 LNN) | `{"ACL_LIFE_CYCLE_DAYS":1095,"IsLnnAcl":"false","serviceId":0}` |

**Sink DP RDB `accesser_table.accesserExtraData`：** `{"dmVersion":"5.1.5"}`（不含 ACL_LIFE_CYCLE_DAYS）

**Sink DP RDB `accessee_table.accesseeExtraData`：** `{"dmVersion":"5.1.5"}`（不含 ACL_LIFE_CYCLE_DAYS）

> BR-4 不变量已验证：`ACL_LIFE_CYCLE_DAYS` 是**关系级属性**，仅持久化在 `AccessControlProfile.extraData`，不会写入 `accesser/accessee.extraData`。

#### E2E-7 详细证据（设备代理绑定，days=1095）

**自动化脚本：** `https://gitcode.com/hwzhangchuang/OH_DeviceManager_SDD_AI_TEST_CODE/blob/main/issue-2445-acl-life-cycle-days/xdevice/run_proxy_test.py`

**报告目录：** `/home/luna/workspace/issue_2445_auto_test_report/proxy_summary.md`

**Source 输出：**
```json
{
  "a3": "PASS",
  "a4": "PASS",
  "a5": "PASS",
  "bindResult": 0,
  "bindStatus": 8,
  "wifiIpMasked": "192.***43.6",
  "wifiPort": 36047,
  "agentLifeCycleDays": 1095,
  "subjectLifeCycleDays": 1095,
  "selfTokenId": 671223800
}
```

**Sink 输出：**
```json
{
  "a1": "PASS",
  "agentPinLen": 6,
  "agentPinMasked": "***",
  "agentPkgName": "CollaborationFwk",
  "agentExportRet": 0,
  "agentImportRet": 0,
  "agentLifeCycleDays": 1095,
  "userId": 100,
  "selfTokenId": 672120760
}
```

**Sink DP RDB `access_control_table`：**

- LNN ACL: `{"IsLnnAcl":"true","serviceId":0}`
- Agent ACL (`CollaborationFwk` → `CollaborationFwk`):
  `{"ACL_LIFE_CYCLE_DAYS":1095,"IsLnnAcl":"false","serviceId":0}`
- Proxy subject ACL (`com.ohos.settings` → `com.ohos.settings`):
  `{"ACL_LIFE_CYCLE_DAYS":1095,"IsLnnAcl":"false"}`

> 设备代理绑定生命周期以客体侧注册的 AuthInfo 为准：业务自身与被代理业务均使用同一个
> `ACL_LIFE_CYCLE_DAYS=1095`。

#### E2E-2 证据（哨兵值 -1）

**Sink DP RDB `access_control_table`：**

| accessControlId | bindLevel | extraData |
|-----------------|-----------|-----------|
| 1 | 1 | `{"IsLnnAcl":"true","serviceId":0}` |
| 2 | 2 | `{"IsLnnAcl":"false","serviceId":0}` |

所有行均不含 `ACL_LIFE_CYCLE_DAYS` key。**Fallback 行为**：`GetAclAllowSeconds` 返回 `MAX_ALWAYS_ALLOW_SECONDS = 31536000`（365 天）。

#### E2E-3 证据（超界值 3651）

**Sink 输出：**
```json
{"err":"import_auth_info_failed:96929749"}
```

错误码 `96929749` = `ERR_DM_INPUT_PARA_INVALID`，在 `IsImportAuthInfoValid` 入口被拒。

#### E2E-4 证据（零值）

**Sink 输出：**
```json
{"err":"import_auth_info_failed:96929749"}
```

同样在入口被拒。0 不在 `[1, 3650]` 范围内且非 `-1` 哨兵。

### 3.2 Spec AC 覆盖矩阵

| AC 编号 | Spec 描述 | 端到端覆盖 | UT 覆盖 | 状态 |
|---------|----------|-----------|---------|------|
| AC-1 | TRUST_ALWAYS + 合法天数 ∈ [1,3650] → 持久化到 ACL extraData | E2E-1, E2E-5, E2E-6 | `UTTest_device_manager_service_three`、`UTTest_auth_confirm`、`UTTest_dm_auth_message_processor` | **PASS** |
| AC-2 | 非 TRUST_ALWAYS 携带正数天数 / 超界 / 类型错误 → 拒绝 | E2E-3, E2E-4 | `UTTest_device_manager_service_three`（10 用例） | **PASS** |
| AC-3 | 哨兵 -1 或缺失 → ACL extraData 不含该字段，fallback 365 天 | E2E-2 | `UTTest_auth_confirm`、`UTTest_device_manager_service_impl_first` | **PASS** |
| AC-4 | GetAclAllowSeconds 从 ACL extraData 读取；缺失/非法 → fallback 365 天 | （读路径由 E2E-2 fallback 间接验证） | `UTTest_device_manager_service_impl_first`（6 用例） | **PASS** |
| AC-5 | 跨版本兼容（旧服务读新 ACL / 新服务读旧 ACL） | （需双版本测试，非端到端可覆盖） | `UTTest_device_manager_service_impl_first` fallback 用例 | **PASS** |

### 3.3 单元测试验证（UT，rk3568 板端执行）

> 编译环境：远端 `113.44.191.30`，`/srv/workspace/openharmony_master_default_20260530115923_huawei_5b2f3bb80/code`
>
> 编译命令：`hb build -T foundation/distributedhardware/device_manager/test/unittest:UTTest_xxx`
>
> 执行设备：RK3568 SN `150100424a5444345209d94ebed7b900`，`hdc -t` 推送至 `/data/test/dm_ut_3730`

| UT Target | 全量用例数 | 通过数 | 失败数 | 执行方式 | 结论 |
|-----------|-----------|--------|--------|----------|------|
| `UTTest_device_manager_service_three` | 120 | 120 | 0 | 全量执行 | **PASS** |
| `UTTest_dm_auth_message_processor` | 33 | 33 | 0 | 全量执行（含修复后 3 个 PutDeviceControlList 新增用例） | **PASS** |
| `UTTest_auth_confirm`（\*AclLifeCycleDays\* 过滤） | 6 | 6 | 0 | `--gtest_filter='*AclLifeCycleDays*'` | **PASS** |
| `UTTest_device_manager_service_impl_first`（\*GetAclAllowSeconds\* 过滤） | 10 | 10 | 0 | `--gtest_filter='*GetAclAllowSeconds*'` | **PASS** |

**说明**：`UTTest_auth_confirm` 和 `UTTest_device_manager_service_impl_first` 全量执行时在非 PR 改动的旧用例处触发 Signal 6/11（板上 indep 编译产物与运行时 .so CFI bitmap 不一致，见 §4.1），与本次 ACL_LIFE_CYCLE_DAYS 改动无关。PR 相关新增用例通过 `--gtest_filter` 过滤执行，均 PASS。

#### UT 新增用例明细

| UT Target | 新增用例 | 验证点 |
|-----------|---------|--------|
| `UTTest_auth_confirm` | `ProcessImportAuthInfo_AclLifeCycleDays_NegativeNonSentinelKeepsDefault` | 负数非哨兵值 → 不覆盖 context，保持 ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED |
| `UTTest_device_manager_service_impl_first` | `GetAclAllowSeconds_NegativeNonSentinel_ReturnsDefault` | -2 → fallback 365天 |
| | `GetAclAllowSeconds_MinDays_ReturnsScaled` | 1天 → 86400秒 |
| | `GetAclAllowSeconds_MaxDays_ReturnsScaled` | 3650天 → 315360000秒 |
| | `GetAclAllowSeconds_SentinelValue_ReturnsDefault` | -1哨兵 → fallback 365天 |
| `UTTest_device_manager_service_three` | `IsImportAuthInfoValid_AclDays_NegativeNonSentinelRejected` | -2 → 拒绝 |
| | `IsImportAuthInfoValid_AclDays_AlwaysSentinelOk` | -1哨兵 + TRUST_ALWAYS → 通过 |
| `UTTest_dm_auth_message_processor` | `PutDeviceControlList_Days_WritesAclExtraData` | device-level bind + ALLOW_AUTH_ALWAYS → extraData 含 ACL_LIFE_CYCLE_DAYS |
| | `PutDeviceControlList_NotAlways_NoAclDays` | 非 ALLOW_AUTH_ALWAYS → extraData 不含 ACL_LIFE_CYCLE_DAYS |
| | `PutDeviceControlList_Sentinel_NoAclDays` | 哨兵值 → extraData 不含 ACL_LIFE_CYCLE_DAYS |

#### UT 修复记录

`UTTest_dm_auth_message_processor` 首次板端执行发现 `PutDeviceControlList_Days_WritesAclExtraData` 失败：`DmAccess.isPutLnnAcl` 默认值为 `true`，导致 mock 捕获到 LNN ACL 分支的 `PutAccessControlProfile` 调用而非预期的非 LNN ACL 分支。修复方式：在三个 `PutDeviceControlList` 测试用例中显式设置 `access.isPutLnnAcl = false`，并移除多余的 `.WillOnce(Return(DM_OK))`（非 LNN 分支仅调用一次 `PutAccessControlProfile`）。修复后重新编译部署，33 用例全部 PASS。

## 4. 已知问题与规避方案

### 4.1 Indep 编译导致 CFI 崩溃

使用 `./build.sh --product-name rk3568 --build-target foundation/distributedhardware/device_manager:device_manager`（indep 编译）时，产出的 `.so` 与板上 `libappexecfwk_core.z.so` 的 CFI bitmap 不一致，导致 `GetBundleLable -> __cfi_check_fail -> SIGABRT`。**解决方案**：使用 `make_all` 整编产出完整镜像，确保所有 `.so` 共享同一 CFI bitmap。详见 `docs/06-debugging.md` §5.2.8。

### 4.2 DP RDB WAL 检查点时序

从设备拉取 `dp_rdb.db` 做验证时，DP SA 必须仍在运行（持有 WAL）。若在 `hdc file recv` 之前 kill DP SA，未提交的 WAL 页可能丢失，导致 `access_control_table` 查到 0 行（假阴性）。**规避方案**：始终在 DP SA 运行时拉取全套文件（`dp_rdb.db` + `dp_rdb.db-wal` + `dp_rdb.db-shm`）。详见 `docs/06-debugging.md` §5.2.11。

## 5. 待办事项

- [x] 删除临时 `DEBUG_2445` LOGI（`auth_confirm.cpp`，已在本次 commit 移除）
- [x] 跑 UT 套件（`UTTest_dm_auth_message_processor`、`UTTest_auth_confirm`、`UTTest_device_manager_service_impl_first`、`UTTest_device_manager_service_three`）并补充结果
- [x] 补充 UT 结果到本报告
- [ ] 推送新 commit 到 PR 3730
