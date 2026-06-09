# DM 开源/闭源隔离宏机制

**版本**：v1.0
**更新日期**：2026-05-30

## 1. 概述

DM 仓存在开源版本和闭源（商业）版本的差异。闭源版本包含 vendor 私有的 `distributed_hardware_adapter` 组件及 `libdevicemanagerresident.z.so`，提供单设备/双设备兼容适配、服务注册/发现扩展、设备图标等闭源能力；开源版本（如 RK3568 标准镜像）不含这些组件。

为在同一份源码中同时支持两种版本，DM 使用 **`DEVICE_MANAGER_COMMON_FLAG`** 编译宏做隔离：

- **宏存在**（`#ifdef DEVICE_MANAGER_COMMON_FLAG`）= **开源版本**
- **宏不存在** = **闭源版本**（含 vendor adapter resident so）

## 2. 构建门控

### 2.1 GN 变量

`device_manager.gni:91-97`：

```gni
if (defined(global_parts_info) &&
    defined(global_parts_info.distributedhardware_distributed_hardware_adapter)) {
  device_manager_common = false   # 闭源：adapter 组件存在
} else {
  device_manager_common = true    # 开源：adapter 组件不存在
}
```

### 2.2 宏注入

当 `device_manager_common = true` 时，以下 BUILD.gn 目标注入宏定义：

| BUILD.gn 位置 | 目标 | defines |
|---|---|---|
| `services/service/BUILD.gn:321-322` | devicemanagerservice | `DEVICE_MANAGER_COMMON_FLAG` |
| `services/service/BUILD.gn:479-480` | devicemanagerservice_test | `DEVICE_MANAGER_COMMON_FLAG` |
| `services/implementation/BUILD.gn:266-267` | devicemanagerserviceimpl | `DEVICE_MANAGER_COMMON_FLAG` |

## 3. 宏使用清单

### 3.1 跳过 vendor resident so 加载/调用

**`services/service/src/device_manager_service.cpp`** 中有多处 `#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE)) && !defined(DEVICE_MANAGER_COMMON_FLAG)` 守卫，用于控制依赖 `dmServiceImplExtResident_`（即 `libdevicemanagerresident.z.so`）的代码块：

| 位置 | 守卫模式 | 保护的代码 |
|---|---|---|
| `device_manager_service.cpp` resident so 调用点 | `!LITE && !COMMON_FLAG` / `#ifndef DEVICE_MANAGER_COMMON_FLAG` | resident so 加载、重启、调用等闭源适配逻辑 |
| `ImportAuthInfo` 第二段 | `#ifndef DEVICE_MANAGER_COMMON_FLAG` | 闭源版调用 resident so；开源版直接 `return DM_OK` |
| `ExportAuthInfo` 第二段 | `#ifndef DEVICE_MANAGER_COMMON_FLAG` | 闭源版调用 resident so；开源版直接 `return DM_OK` |
| 其他 resident so 相关代码 | `#if !defined(DEVICE_MANAGER_COMMON_FLAG)` | 闭源能力分支 |

**头文件 `services/service/include/device_manager_service.h`**：

| 声明 | 守卫模式 | 说明 |
|---|---|---|
| resident so 相关成员与加载状态 | 按具体成员使用场景最小范围守卫 | 闭源 resident so 依赖项不应泄漏到开源调用路径 |
| `IsDMServiceAdapterResidentLoad()` 等辅助函数 | 按调用点和实现最小范围守卫 | 开源路径通过 `DEVICE_MANAGER_COMMON_FLAG` 跳过第二段调用 |

### 3.2 认证状态机分支

**`services/implementation/src/authentication_v2/`**：

| 文件 | 行号 | 守卫模式 | 作用 |
|---|---|---|---|
| `dm_auth_manager_base.cpp` | L551 | `#ifdef DEVICE_MANAGER_COMMON_FLAG` | 开源版走简化分支（跳过闭源扩展逻辑） |
| `auth_stages/auth_acl.cpp` | L36 | `#ifdef DEVICE_MANAGER_COMMON_FLAG` | ACL 阶段开源版分支 |
| `auth_stages/auth_confirm.cpp` | L704 | `#ifdef DEVICE_MANAGER_COMMON_FLAG` | confirm 阶段开源版分支 |

### 3.3 老协议分支

| 文件 | 行号 | 守卫模式 | 作用 |
|---|---|---|---|
| `authentication/dm_auth_manager.cpp` | L3240 | `#ifndef DEVICE_MANAGER_COMMON_FLAG` | 闭源版独占逻辑 |

### 3.4 单测

| 文件 | 行号 | 守卫模式 | 作用 |
|---|---|---|---|
| `test/unittest/UTTest_device_manager_service_two.cpp` | L1501 | `#if !defined(DEVICE_MANAGER_COMMON_FLAG)` | 仅闭源版编译的测试用例 |

## 4. 关键设计：ExportAuthInfo/ImportAuthInfo 的两段式执行

`device_manager_service.cpp` 中 `ExportAuthInfo` 和 `ImportAuthInfo` 采用两段式执行：

```
第一段（开源）：dmServiceImpl_->ExportAuthInfo() / dmServiceImpl_->ImportAuthInfo()
  ↓ 生成 PIN / 写入 LocalServiceInfo 到 DP
第二段（闭源）：dmServiceImplExtResident_->ExportAuthInfo() / ImportAuthInfo()
  ↓ vendor 私有同步/上报
```

**开源版本**（`DEVICE_MANAGER_COMMON_FLAG` 定义时）：第二段不存在，宏隔离直接 `return DM_OK`；其中 `ExportAuthInfo` 需要 `DM_OK` 才能让 IPC stub 正常 encode `DmAuthInfo` 回传给 SDK 调用方。

**闭源版本**：两段都执行；第二段失败返回 `ERR_DM_UNSUPPORTED_METHOD (96929776)`；其中 `ExportAuthInfo` 的 IPC stub 不回传 `DmAuthInfo`。

### 为什么开源版 `ExportAuthInfo` 不能简单"忽略 errno"

`ExportAuthInfo` 的 IPC stub（`services/service/src/ipc/standard/ipc_cmd_parser.cpp:991` 和 `interfaces/inner_kits/native_cpp/src/ipc/standard/ipc_cmd_parser.cpp:1071`）在 `result != DM_OK` 时**不 encode/decode `DmAuthInfo`**：

```cpp
// service 侧 IPC stub
if (result == DM_OK && !IpcModelCodec::EncodeDmAuthInfo(authInfo, reply)) { ... }

// SDK 侧 IPC reader
if (pBaseRsp->GetErrCode() == DM_OK) {
    IpcModelCodec::DecodeDmAuthInfo(reply, info);
    pRsp->SetDmAuthInfo(info);
}
```

因此开源版 `ExportAuthInfo` **必须在 service 侧就返回 `DM_OK`**，而非让调用方忽略 errno——否则 `DmAuthInfo`（含 PIN）不会通过 IPC 回传。`ImportAuthInfo` 只回传错误码，也应返回 `DM_OK` 以表达开源段处理成功。

## 5. 已知开源镜像验证限制

### 5.1 服务代理绑定依赖闭源 BLE 服务字段

服务绑定 / 服务代理绑定的发现链路依赖软总线 BLE 广播中携带服务字段。该字段能力由闭源
vendor 组件补齐，RK3568 开源标准镜像不包含对应实现，因此蓝区开源板上只能稳定验证设备
绑定与设备代理绑定，不能把服务绑定 / 服务代理绑定作为开源镜像端到端验收项。

## 6. 新增代码时的判断规则

| 涉及的代码/资源 | 判断 | 宏用法 |
|---|---|---|
| `libdevicemanagerresident.z.so` 相关（dlopen/dlsym/调用） | 闭源专属 | `#ifndef DEVICE_MANAGER_COMMON_FLAG` 包住 |
| `distributed_hardware_adapter` 组件接口 | 闭源专属 | `#ifndef DEVICE_MANAGER_COMMON_FLAG` 包住 |
| 仅闭源设备形态才有的业务逻辑 | 闭源专属 | `#ifdef DEVICE_MANAGER_COMMON_FLAG` 提供开源 fallback |
| 同时适用于开源和闭源的逻辑 | 不需宏 | 不加守卫 |

**原则**：
- 宏守卫应放在**最小必要范围**，不扩大隔离边界
- 开源 fallback 路径必须与闭源路径的功能差异**在文档中显式声明**
- 修改已有宏守卫代码时，必须**同步验证开源版和闭源版编译**
