# DeviceManager 核心概念与术语

## 1. 设备标识

- `deviceId`：对外接口常用设备标识。
- `networkId`：SoftBus 网络内临时通信标识。
- `udid` / `uuid`：内部认证、ACL、设备身份相关标识。

这些标识均视为敏感信息，不得明文落日志、事件、PR、issue。

## 2. 可信关系与 ACL

ACL 表示一条可信关系。关系级属性必须写入 `AccessControlProfile.extraData`；实体级属性才写入 accesser/accessee。

`ACL_LIFE_CYCLE_DAYS` 是单条可信关系的生命周期，不是设备属性。同一设备可存在多条不同服务或绑定类型的 ACL，每条 ACL 生命周期独立。

## 3. 永久可信关系生命周期

- 生命周期判断以单条 ACL 上的关系级属性为准。
- 指定生命周期不得被全局默认值覆盖。
- 清理逻辑必须逐条 ACL 判断。

## 4. 认证协议边界

`authentication_v2` 与老协议互不交叉；新增认证能力默认走 v2，不主动改老协议。

## 5. JSON 与 extraData

新增关系属性优先走 JSON `extraData`，不得改变 DP schema；解析 JSON 先判 `IsDiscarded`。
