# services/implementation 指引

> 本文件适用于 `services/implementation/`。根目录 `AGENTS.md` 仍是知识库总入口，本文件只补充服务实现侧规则。

## 任务分类

- 认证、绑定、解绑、ACL 写入、发现、广告、HiChain 集成等业务主干默认从本目录开始。
- 新协议优先看 `src/authentication_v2/`；老协议 `src/authentication/` 默认只做兼容 bugfix。
- 设备生命周期、ACL 清理、永久可信策略优先看 `src/device_manager_service_impl.cpp`。

## 必读路径

| 场景 | 先读 |
|---|---|
| 改绑定/解绑状态机 | `src/authentication_v2/` + `docs/04-workflows/07-bind-unbind.md` |
| 改导入 PIN 免弹窗绑定 | `docs/04-workflows/08-import-pin-no-popup-bind.md` |
| 改 ACL 写入 / 清理 | `docs/04-workflows/03-trust-management.md` + `src/device_manager_service_impl.cpp` |
| 改 DP/HiChain/SoftBus 交互 | `docs/05-interfaces/03-dependency-api.md` |
| 改权限或调用方校验 | `docs/04-workflows/05-permission-check.md` |

## 约束

- sink/source 分支必须职责清晰，跨角色改动要同时说明两端影响。
- 关系属性只写 `AccessControlProfile.extraData`，不要写入 `accesser/accessee.extraData`。
- 不改 DP schema；新增关系级字段走 ACL `extraData` JSON 扩展位。
- JSON 解析必须使用 `json/json_object.h`，先判断 `IsDiscarded`，不要直接 include nlohmann/cJSON。
- 不使用 `std::stoi`、try-catch 或依赖异常的控制流。
- 不把 `authentication_v2` helper 抄进 `src/authentication/` 老协议路径。
- 不明文打印 `networkId`、`udid`、`uuid`、`deviceId`、PIN 或位置信息。

## 验证

- C++ 改动至少编译 `device_manager`。
- 认证/ACL 改动还要编译 `device_manager_test`，并优先跑关联 UT。
- ACL 写入 / 清理至少关注 `UTTest_dm_auth_message_processor` 和 `UTTest_device_manager_service_impl_first`。
- 代码风格自检：单行不超过 120 字符，新增数值必须使用具名常量。
