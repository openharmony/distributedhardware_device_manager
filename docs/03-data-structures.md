# DeviceManager 数据结构约束

## 1. 记录原则

本文件只记录跨结构的语义约束；字段列表、枚举值、函数签名以源码和公开头文件为准，不在文档中复制。

## 2. 关系属性与实体属性

DP 持久化模型中 ACL、accesser、accessee 语义不同：

| 对象 | 语义 | extraData 用途 |
|---|---|---|
| `AccessControlProfile` | 一条可信关系本身 | 关系级属性，如 `ACL_LIFE_CYCLE_DAYS`、`IsLnnAcl`、`serviceId` |
| `Accesser` | 关系主体侧实体 | 实体级身份补充 |
| `Accessee` | 关系客体侧实体 | 实体级身份补充 |

`ACL_LIFE_CYCLE_DAYS` 是单条可信关系的生命周期，不是设备属性；必须写入 `AccessControlProfile.extraData`，不得写入 `accesser/accessee.extraData`。

## 3. JSON 约束

- `LocalServiceInfo`、`ImportAuthInfo`、`extraInfo` 等 JSON 输入解析后先判 `IsDiscarded`。
- 全仓 `-fno-exceptions`，不得用 try-catch 或 `std::stoi` 处理输入。
- `JsonObject` 复制使用 `Duplicate()`。

## 4. 敏感字段

`networkId`、`udid`、`uuid`、`deviceId`、PIN、位置信息不得明文写入日志、事件、PR 或 issue。
