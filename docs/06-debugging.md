# DeviceManager 调试与故障排查指南

**版本：v2.0**  
**更新日期：2026-05-19**

---

## 1. 概述

本文档提供 OpenHarmony DeviceManager 的调试方法、工具使用和常见故障排查流程。DeviceManager 作为分布式设备管理的核心服务，涉及设备发现、认证、信任关系管理等多个复杂流程，因此掌握系统的调试方法对于快速定位和解决问题至关重要。

### 1.1 调试体系架构

DeviceManager 的调试体系包含以下层次：

```
┌─────────────────────────────────────────────────────────┐
│                  应用层调试                               │
│  (JS/Native API 调用、回调、状态变化)                      │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│                  服务层调试                               │
│  (DeviceManagerService、IPC 通信、权限管理)                │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│                  实现层调试                               │
│  (认证、发现、状态管理等核心逻辑)                          │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│                  依赖模块调试                              │
│  (SoftBus、HiChain、DP、账号)                             │
└─────────────────────────────────────────────────────────┘
```

### 1.2 调试工具链

- **Hilog**：系统日志工具，用于查看实时日志
- **HiSysEvent**：系统事件打点，用于追踪关键行为和故障
- **Radar**：性能监控和行为分析工具
- **HDC**：HarmonyOS 设备调试桥，用于命令行调试

---

## 2. 日志配置

### 2.1 日志标签

DeviceManager 使用统一的日志标签体系：

| 标签 | 模块 | 日志级别范围 | 说明 |
|------|------|-------------|------|
| `DHDM` | 全局 | ALL | DeviceManager 主标签，所有模块共用 |
| `DH_DM_SERVER` | 服务层 | DEBUG-ERROR | DeviceManagerService 相关 |
| `DH_DM_IPC` | IPC通信 | DEBUG-ERROR | IPC 客户端/服务端通信 |
| `DH_DM_AUTH` | 认证模块 | DEBUG-ERROR | 设备认证、HiChain 交互 |
| `DH_DM_DISCOVER` | 发现模块 | DEBUG-ERROR | 设备发现、发布订阅 |
| `DH_DM_STATE` | 状态管理 | DEBUG-ERROR | 设备状态、信任关系 |
| `DH_DM_SOFTBUS` | SoftBus适配 | DEBUG-ERROR | SoftBus 连接管理 |
| `DH_DM_DP` | DP适配 | DEBUG-ERROR | DeviceProfile 交互 |
| `DH_DM_ACL` | 权限管理 | DEBUG-ERROR | ACL、权限控制 |

**日志标签定义位置**：
- 主标签：`common/include/dm_log.h`
- 三方标签：`3rd/utils/include/dm_log_3rd.h`

### 2.2 日志级别

DeviceManager 使用 Hilog 的标准日志级别：

| 级别 | 宏定义 | 使用场景 | 示例 |
|------|--------|----------|------|
| **DEBUG** | `LOGD()` | 详细的调试信息，仅在开发阶段使用 | 函数入口、参数值、中间状态 |
| **INFO** | `LOGI()` | 关键流程节点，用于追踪正常流程 | 流程开始/结束、状态变化 |
| **WARN** | `LOGW()` | 异常但可恢复的情况 | 重试、降级、兼容性处理 |
| **ERROR** | `LOGE()` | 错误和异常情况 | 函数失败、参数错误、返回错误 |

**日志宏定义**：
```cpp
#include "dm_log.h"

// 基础日志宏
LOGD("This is a debug log: %{public}s", msg.c_str());
LOGI("Device discovered: %{public}s", deviceId.c_str());
LOGW("Retry authentication, attempt %{public}d", retryCount);
LOGE("Failed to open session: %{public}d", errCode);

// 检查宏
CHECK_NULL_VOID(pointer);           // 检查空指针，为空则返回
CHECK_NULL_RETURN(pointer, ret);    // 检查空指针，为空则返回错误码
CHECK_EMPTY_VOID(container);        // 检查容器为空
CHECK_SIZE_RETURN(container, ret);  // 检查容器大小超限
```

### 2.3 日志开关与动态调整

#### 2.3.1 编译时日志控制

通过 BUILD.gn 配置日志开关：

```gn
# services/service/BUILD.gn
ohos_shared_library("device_manager_service") {
  defines = [
    "HILOG_ENABLE",      # 启用 Hilog
    "DM_DH_LOG_TAG=\"DH_DM_SERVER\"",  # 子标签
  ]
}
```

#### 2.3.2 运行时日志级别控制

使用 hdc shell 命令动态调整日志级别：

```bash
# 设置全局日志级别为 DEBUG（显示所有日志）
hdc shell hilog -r && hdc shell hilog -b D

# 设置 DHDM 标签为 DEBUG 级别
hdc shell hilog -t DHDM -b D

# 设置特定标签为 ERROR 级别
hdc shell hilog -t DH_DM_AUTH -b E

# 查看当前日志配置
hdc shell hilog -Q pidoff
hdc shell hilog -q
```

#### 2.3.3 日志过滤技巧

```bash
# 只看 DeviceManager 相关日志
hdc shell hilog -t DHDM

# 只看 ERROR 级别日志
hdc shell hilog -t DHDM -b E

# 过滤关键字
hdc shell hilog -t DHDM | grep "authentication"

# 清空日志缓冲区
hdc shell hilog -r

# 实时查看日志
hdc shell hilog -t DHDM | grep --line-buffered "device discovered"

# 将日志保存到文件
hdc shell hilog -t DHDM > dm_log.txt
```

### 2.4 关键日志格式说明

DeviceManager 的日志采用统一格式：

```
[级别] [时间] [PID:tid] [标签] [模块][函数]: 消息内容
```

**示例**：
```
05-19 10:23:45.123  1234  5678 I 01234/DHDM: [DH_DM_SERVER][Init]: "DeviceManager service initializing..."
05-19 10:23:45.124  1234  5678 I 01234/DHDM: [DH_DM_SOFTBUS][RegisterLnn]: "SoftBus LNN callback registered"
05-19 10:23:45.125  1234  5678 E 01234/DHDM: [DH_DM_AUTH][CreateGroup]: "Failed to create HiChain group: -20026"
```

**关键字段**：
- `[模块]`：标识子模块，如 `DH_DM_AUTH`
- `[函数]`：当前函数名
- `%{public}s`：Hilog 格式化占位符，表示公开字符串（可被日志工具读取）
- `%{private}s`：私有字符串（会被脱敏处理）

**敏感信息脱敏**：
```cpp
// UDID 等敏感信息会被脱敏
LOGI("Local device: %{public}s", GetAnonyString(localUdid).c_str());
// 输出: Local device: ABC***123
```

---

## 3. HiSysEvent 打点

HiSysEvent 是 OpenHarmony 的系统事件记录机制，DeviceManager 使用它记录关键行为和故障。

### 3.1 事件定义

DeviceManager 定义的 HiSysEvent 详见 `hisysevent.yaml`：

| 事件名称 | 类型 | 级别 | 说明 |
|---------|------|------|------|
| **DM_INIT_DEVICE_MANAGER_SUCCESS** | BEHAVIOR | CRITICAL | DeviceManager 初始化成功 |
| **DM_INIT_DEVICE_MANAGER_FAILED** | BEHAVIOR | CRITICAL | DeviceManager 初始化失败 |
| **START_DEVICE_DISCOVERY_SUCCESS** | BEHAVIOR | CRITICAL | 设备发现启动成功 |
| **START_DEVICE_DISCOVERY_FAILED** | BEHAVIOR | CRITICAL | 设备发现启动失败 |
| **GET_LOCAL_DEVICE_INFO_SUCCESS** | BEHAVIOR | CRITICAL | 获取本地设备信息成功 |
| **GET_LOCAL_DEVICE_INFO_FAILED** | BEHAVIOR | CRITICAL | 获取本地设备信息失败 |
| **DM_SEND_REQUEST_SUCCESS** | BEHAVIOR | CRITICAL | 发送请求成功 |
| **DM_SEND_REQUEST_FAILED** | BEHAVIOR | CRITICAL | 发送请求失败 |
| **ADD_HICHAIN_GROUP_SUCCESS** | BEHAVIOR | CRITICAL | 添加 HiChain 群组成员成功 |
| **ADD_HICHAIN_GROUP_FAILED** | BEHAVIOR | CRITICAL | 添加 HiChain 群组成员失败 |
| **DM_CREATE_GROUP_SUCCESS** | BEHAVIOR | CRITICAL | 创建 HiChain 群组成功 |
| **DM_CREATE_GROUP_FAILED** | BEHAVIOR | CRITICAL | 创建 HiChain 群组失败 |
| **UNAUTHENTICATE_DEVICE_SUCCESS** | BEHAVIOR | CRITICAL | 删除信任关系成功 |
| **UNAUTHENTICATE_DEVICE_FAILED** | BEHAVIOR | CRITICAL | 删除信任关系失败 |

**事件定义位置**：`hisysevent.yaml`
**事件打点实现**：`common/src/dfx/standard/dm_hisysevent.cpp`

### 3.2 事件参数说明

所有 HiSysEvent 包含以下通用参数：

| 参数名 | 类型 | 说明 |
|--------|------|------|
| **PID** | INT32 | 进程 ID |
| **UID** | INT32 | 用户 ID |
| **MSG** | STRING | 事件详细消息 |

**示例事件记录**：
```cpp
// 记录认证失败事件
HiSysEventParam params[] = {
    {.name = "PID", .t = HISYSEVENT_INT32, .v = { .i32 = getpid() } },
    {.name = "UID", .t = HISYSEVENT_INT32, .v = { .i32 = getuid() } },
    {.name = "MSG", .t = HISYSEVENT_STRING, .v = { .s = (char *)"Failed to authenticate device" } },
};
OH_HiSysEvent_Write(
    HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
    "UNAUTHENTICATE_DEVICE_FAILED",
    HISYSEVENT_BEHAVIOR, params, sizeof(params) / sizeof(params[0]));
```

### 3.3 查询方式

#### 3.3.1 使用 HiSysEvent 查询工具

```bash
# 查询 DeviceManager 相关的所有事件
hdc shell hidumper -s 1202 -a "-domain DISTDM"

# 查询认证失败事件
hdc shell hidumper -s 1202 -a "-domain DISTDM -event UNAUTHENTICATE_DEVICE_FAILED"

# 查询最近 100 条事件
hdc shell hidumper -s 1202 -a "-domain DISTDM -max 100"

# 查询指定时间范围的事件
hdc shell hidumper -s 1202 -a "-domain DISTDM -begin 2026-05-19 10:00:00 -end 2026-05-19 11:00:00"
```

#### 3.3.2 使用 HiSysEvent C++ API

```cpp
#include "hisysevent.h"

// 订阅 HiSysEvent
OH_HiSysEvent_AddEventListener(
    HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
    "DM_INIT_DEVICE_MANAGER_FAILED",
    [](const HiSysEventRecord& record) {
        // 处理事件
        LOGI("HiSysEvent: %{public}s", record eventName_.c_str());
    }
);
```

#### 3.3.3 使用 HiSysEvent JS API

```javascript
import hiSysEvent from '@ohos.hiSysEvent';

// 查询事件
hiSysEvent.query({
    domain: "DISTDM",
    eventName: "UNAUTHENTICATE_DEVICE_FAILED",
    maxEvents: 100
}, (err, data) => {
    if (err) {
        console.error("Query failed:", err);
        return;
    }
    console.log("Events:", data);
});
```

---

## 4. Radar 打点

Radar 是 OpenHarmony 的性能监控和行为分析工具，DeviceManager 使用 Radar 进行细粒度的行为追踪。

### 4.1 Radar 事件定义

DeviceManager 定义了以下 Radar 业务场景：

| 业务场景 | 枚举值 | 说明 |
|---------|--------|------|
| **DM_DISCOVER** | 1 | 设备发现 |
| **DM_GET_TRUST_DEVICE_LIST** | 2 | 获取可信设备列表 |
| **DM_GET_LOCAL_DEVICE_INFO** | 3 | 获取本地设备信息 |
| **DM_GET_DEVICE_INFO** | 4 | 获取对端设备信息 |
| **DM_BEHAVIOR** | 5 | 通用行为记录 |

#### 4.1.1 设备发现阶段

| 阶段 | 枚举值 | 说明 |
|------|--------|------|
| **DISCOVER_REGISTER_CALLBACK** | 1 | 注册发现回调 |
| **DISCOVER_USER_DEAL_RES** | 2 | 用户处理发现结果 |
| **DISCOVER_GET_TRUST_DEVICE_LIST** | 3 | 获取可信设备列表 |

#### 4.1.2 认证阶段

| 阶段 | 枚举值 | 说明 |
|------|--------|------|
| **AUTH_START** | 1 | 开始认证 |
| **AUTH_OPEN_SESSION** | 2 | 打开会话 |
| **AUTH_SEND_REQUEST** | 3 | 发送认证请求 |
| **AUTH_PULL_AUTH_BOX** | 4 | 拉起认证确认框 |
| **AUTH_CREATE_HICHAIN_GROUP** | 5 | 创建 HiChain 群组 |
| **AUTH_PULL_PIN_BOX_START** | 6 | 拉起 PIN 码输入框（开始） |
| **AUTH_PULL_PIN_INPUT_BOX_END** | 7 | 拉起 PIN 码输入框（结束） |
| **AUTH_ADD_HICHAIN_GROUP** | 8 | 添加 HiChain 群组成员 |

#### 4.1.3 网络阶段

| 阶段 | 枚举值 | 说明 |
|------|--------|------|
| **NETWORK_ONLINE** | 1 | 设备上线 |
| **NETWORK_OFFLINE** | 2 | 设备下线 |

#### 4.1.4 其他场景

| 枚举 | 枚举值 | 说明 |
|------|--------|------|
| **DELETE_TRUST** | 1 | 删除信任关系 |
| **GET_TRUST_DEVICE_LIST** | 1 | 获取可信设备列表 |
| **CREATE_PIN_HOLDER** | 1 | 创建 PIN 码持有者 |
| **SESSION_OPENED** | 2 | 会话已打开 |
| **SEND_CREATE_PIN_HOLDER_MSG** | 3 | 发送创建 PIN 码持有者消息 |
| **RECEIVE_CREATE_PIN_HOLDER_MSG** | 4 | 接收创建 PIN 码持有者消息 |
| **DESTROY_PIN_HOLDER** | 5 | 销毁 PIN 码持有者 |
| **RECEIVE_DESTROY_PIN_HOLDER_MSG** | 6 | 接收销毁 PIN 码持有者消息 |

### 4.1.5 阶段结果

| 结果 | 枚举值 | 说明 |
|------|--------|------|
| **STAGE_IDLE** | 0 | 阶段空闲 |
| **STAGE_SUCC** | 1 | 阶段成功 |
| **STAGE_FAIL** | 2 | 阶段失败 |
| **STAGE_CANCEL** | 3 | 阶段取消 |
| **STAGE_UNKNOW** | 4 | 未知结果 |

### 4.1.6 业务状态

| 状态 | 枚举值 | 说明 |
|------|--------|------|
| **BIZ_STATE_START** | 1 | 业务开始 |
| **BIZ_STATE_END** | 2 | 业务结束 |
| **BIZ_STATE_CANCEL** | 3 | 业务取消 |

### 4.1.7 信任状态

| 状态 | 枚举值 | 说明 |
|------|--------|------|
| **NOT_TRUST** | 0 | 未信任 |
| **IS_TRUST** | 1 | 已信任 |

### 4.1.8 API 类型

| 类型 | 枚举值 | 说明 |
|------|--------|------|
| **API_UNKNOW** | 0 | 未知 API |
| **API_JS** | 1 | JS API |
| **API_NATIVE** | 2 | Native API |

### 4.1.9 通信服务

| 类型 | 枚举值 | 说明 |
|------|--------|------|
| **NOT_USE_SOFTBUS** | 0 | 不使用 SoftBus |
| **USE_SOFTBUS** | 1 | 使用 SoftBus |

### 4.1.10 模块

| 模块 | 枚举值 | 说明 |
|------|--------|------|
| **DEVICE_MANAGER** | 0 | DeviceManager |
| **HICHAIN** | 1 | HiChain |
| **SOFTBUS** | 2 | SoftBus |
| **USER** | 3 | 用户 |

### 4.1.11 Radar 事件参数

| 参数名 | 类型 | 说明 |
|--------|------|------|
| **ORG_PKG** | STRING | 源包名（固定为 "deviceManager"） |
| **HOST_PKG** | STRING | 宿主包名 |
| **FUNC** | STRING | 函数名 |
| **API_TYPE** | INT32 | API 类型 |
| **BIZ_SCENE** | INT32 | 业务场景 |
| **BIZ_STAGE** | INT32 | 业务阶段 |
| **STAGE_RES** | INT32 | 阶段结果 |
| **BIZ_STATE** | INT32 | 业务状态 |
| **TO_CALL_PKG** | STRING | 调用目标包名 |
| **COMM_SERV** | INT32 | 通信服务 |
| **ERROR_CODE** | INT32 | 错误码 |

**Radar 定义位置**：`radar/include/dm_radar_helper.h`  
**Radar 实现**：`radar/src/dm_radar_helper.cpp`

---

## 5. hdc 调试命令

hdc（HarmonyOS Device Connector）是 HarmonyOS 的设备调试桥，类似 Android 的 adb。

### 5.1 常用 hdc 命令

#### 5.1.1 服务状态检查

```bash
# 检查 DeviceManager 服务状态
hdc shell ps -ef | grep devicemanager

# 检查服务是否运行
hdc shell sa list | grep devicemanager

# 查看服务详细信息
hdc shell hdc shell hidumper -s 3511 -a "-h"
```

#### 5.1.2 设备列表查询

```bash
# 查询可信设备列表
hdc shell "hidumper -s 3511 -a '--device-list'"

# 查询本地设备信息
hdc shell "hidumper -s 3511 -a '--local-device'"

# 查询设备详情
hdc shell "hidumper -s 3511 -a '--device-info -networkId xxx'"
```

#### 5.1.3 日志捕获

```bash
# 实时查看 DeviceManager 日志
hdc shell hilog -t DHDM

# 清空日志缓冲
hdc shell hilog -r

# 设置日志级别
hdc shell hilog -b D

# 过滤特定模块日志
hdc shell hilog -t DHDM | grep "authentication"

# 保存日志到文件
hdc shell hilog -t DHDM > dm_log.txt
```

#### 5.1.4 HiSysEvent 查询

```bash
# 查询 DeviceManager 所有 HiSysEvent
hdc shell hidumper -s 1202 -a "-domain DISTDM"

# 查询特定事件
hdc shell hidumper -s 1202 -a "-domain DISTDM -event DM_INIT_DEVICE_MANAGER_FAILED"

# 查询最近 100 条事件
hdc shell hidumper -s 1202 -a "-domain DISTDM -max 100"

# 导出事件到文件
hdc shell hidumper -s 1202 -a "-domain DISTDM" > hisysevent.txt
```

#### 5.1.5 设备操作

```bash
# 触发设备发现
hdc shell "bm dump -n PackageName | grep -A 10 'distributedhardware'"

# 模拟设备上线/下线
hdc shell "softbus_client -c"

# 查看网络状态
hdc shell "ifconfig"

# 查看 SoftBus 状态
hdc shell "softbus_client -s"
```

### 5.2 验证 PR / 本地 DM 改动时：必须先推 .so 再跑测试

**Why**：本仓 UT 与独立测试归档仓中的 sink/source/probe demo 都通过 IPC 与 DM SA 通信。它们的 stub/proxy 行为是由**板子镜像里 `/system/lib/` 下安装的 DM .so 决定的**，而不是测试二进制自己编进去的——`devicemanagersdk` 在板子上是动态库，测试二进制只是 dlopen 调用方。

如果板子镜像里的 DM .so 仍是上版本，而你的 PR 改了 DM 行为，你跑测试看到的是**旧 .so 的行为**，得到的结论无效（PASS/FAIL 都不能信）。

**Rule of thumb**：凡是 PR 改了 `services/service` / `services/implementation` / `common` / `interfaces/inner_kits` 下面的 cpp/h，验证前必须按下面流程把对应 .so 推到板子并重启。

#### 5.2.1 识别 PR 影响的 .so

```bash
# 先列出 PR 改动的源文件（排除 test/docs）
cd foundation/distributedhardware/device_manager
git diff --name-only <base>..HEAD -- '*.cpp' '*.h' \
    | grep -v test/ | grep -v openspec/ | grep -v docs/

# 然后通过 ninja 文件反查每个源文件被编进哪个 .so
find out/<product>/src -name '*.ninja' \
    | xargs grep -nr "source_file_part = <修改文件名>"
```

#### 5.2.2 DM 仓四个核心 .so 与对应板上路径

| .so | 由什么源生成 | 板上路径 |
|---|---|---|
| `libdevicemanagerservice.z.so` | `services/service/src/*.cpp` | `/system/lib/libdevicemanagerservice.z.so` |
| `libdevicemanagerserviceimpl.z.so` | `services/implementation/src/**/*.cpp` | `/system/lib/libdevicemanagerserviceimpl.z.so` |
| `libdevicemanagersdk.z.so` | `interfaces/inner_kits/native_cpp/src/*.cpp` + `common/src/dm_constants.cpp` | `/system/lib/platformsdk/libdevicemanagersdk.z.so` |
| `libdevicemanagerutils.z.so` | `utils/src/**/*.cpp` | `/system/lib/libdevicemanagerutils.z.so` |

> SDK lib 在 `/system/lib/platformsdk/` 子目录，不在 `/system/lib/` 顶层——`find /system -name <lib>` 一次性确认。

#### 5.2.3 推 .so + 重启 + 跑测试（完整流程）

```bash
# 1) 编译要推的 .so（用 hb 单 target 编最快）
hb build --ccache \
    -T devicemanagerservice \
    -T devicemanagerserviceimpl \
    -T devicemanagersdk

# 2) 挂载 /system 可写
hdc -t <sn> target mount

# 3) 替换 .so（按实际 PR 影响的 .so 列表来）
hdc -t <sn> file send out/<product>/distributedhardware/device_manager/libdevicemanagerservice.z.so /system/lib/libdevicemanagerservice.z.so
hdc -t <sn> file send out/<product>/distributedhardware/device_manager/libdevicemanagerserviceimpl.z.so /system/lib/libdevicemanagerserviceimpl.z.so
hdc -t <sn> file send out/<product>/distributedhardware/device_manager/libdevicemanagersdk.z.so /system/lib/platformsdk/libdevicemanagersdk.z.so

# 4) 重启板子让 SA 重新加载新 .so
hdc -t <sn> shell sync
hdc -t <sn> shell reboot
hdc -t <sn> wait-for-device
# 等 BOOT_COMPLETED（SA start-on-demand 在引导完成后才能被拉起）
sleep 30

# 5) 推测试二进制
hdc -t <sn> file send out/<product>/distributedhardware/device_manager/<your_test_binary> /data/local/tmp/
hdc -t <sn> shell chmod 755 /data/local/tmp/<your_test_binary>

# 6) 跑测试
hdc -t <sn> shell /data/local/tmp/<your_test_binary>
```

#### 5.2.4 检查 SA 是否健康（区分镜像问题 vs PR 问题）

如果测试报 `ERR_DM_INIT_FAILED (96929747)` 或 `proxy init failed`，先看是不是 DM SA 自己挂了，而不是你的 PR 出问题：

```bash
# DM SA 进程是否在
hdc -t <sn> shell "ps -ef | grep device_manager | grep -v grep"

# 最近是否有 DM SA cppcrash
hdc -t <sn> shell "ls -lt /data/log/faultlog/faultlogger/ | grep device_manager | head -5"

# 看最新一份 cppcrash 的 stack（Reason / 顶部 5 帧）
hdc -t <sn> shell "head -40 /data/log/faultlog/faultlogger/cppcrash-device_manager-*.log" \
    | tail -45
```

如果 stack 里没有你 PR 改动的源文件，就是**板子镜像本身的 bug**（典型如 `libdistributeddata_inner` UBSan abort），需要换/刷镜像，不要怀疑你的 PR。

#### 5.2.5 一次推完两台板子（分布式测试场景）

```bash
for sn in <sn1> <sn2>; do
    hdc -t $sn target mount
    for so_pair in \
        "libdevicemanagerservice.z.so:/system/lib" \
        "libdevicemanagerserviceimpl.z.so:/system/lib" \
        "libdevicemanagersdk.z.so:/system/lib/platformsdk"; do
        lib="${so_pair%:*}"; dir="${so_pair##*:}"
        hdc -t $sn file send out/<product>/distributedhardware/device_manager/$lib "$dir/$lib"
    done
    hdc -t $sn shell sync
    hdc -t $sn shell reboot
done
# 两台同步等 BOOT_COMPLETED
for sn in <sn1> <sn2>; do
    hdc -t $sn wait-for-device
done
sleep 30
```

#### 5.2.6 常见坑

- **只推 SDK 不重启**：SDK 是 dlopen 库，重启后才生效；SA 是常驻进程，**必须重启**
- **少推 utils**：PR 没改 utils 时不需要，但 `common/include/*.h` 内联到 utils 时要重新编 utils（看 ninja 反查为准）
- **推完忘 sync**：写入未落盘就 reboot，等于没推
- **板子状态不一致**：两台板子若一台推过、一台没推，"是否复现" 这个判断就会带误差——同步推同步重启
- **分布式用例必须双端亮屏（不要求解锁）**：DM 的设备发布依赖屏幕事件，`PublishCommonEventCallback` 只有在 Wi-Fi 可用且屏幕亮起时才会发布设备；任一端灭屏都可能导致 source `StartDiscovering` 30s 超时。跑 sink/source demo 前先让两块 RK 都保持亮屏，必要时执行 `power-shell wakeup`

#### 5.2.7 demo / 自研可执行：native_token 必须用动态库（`*_shared`）才能链通 `Restorecon`

- **现象**：独立测试归档仓中的 `ohos_executable` demo（例如 `issue-2445-acl-life-cycle-days/sink/source/p2_probe`）链接报：
  ```
  ld.lld: error: undefined symbol: Restorecon
  >>> referenced by nativetoken.c:283 (../../base/security/access_token/interfaces/innerkits/nativetoken/src/nativetoken.c:283)
  >>>               lto.tmp:(ClearOrCreateCfgFile)
  >>> referenced by nativetoken.c:802 (../../base/security/access_token/interfaces/innerkits/nativetoken/src/nativetoken.c:802)
  >>>               lto.tmp:(GetAccessTokenId)
  ```
- **根因**：
  - `Restorecon` 由 `base/security/selinux_adapter:librestorecon` 提供（声明在 `interfaces/policycoreutils/include/policycoreutils.h`）
  - `access_token:libnativetoken` 是**静态库**（`libnativetoken.a`），它把对 `Restorecon` 的引用打包但**不携带 selinux_adapter 链接**，需要使用方自己显式带上
  - `access_token:libtoken_setproc` 同样是静态库（`libtoken_setproc.a`），有同类问题
- **修复**：把 external_deps 改成 `_shared` 后缀的动态版本——
  ```gn
  external_deps = [
    "access_token:libaccesstoken_sdk",
    "access_token:libnativetoken_shared",    # 不要用 libnativetoken（静态）
    "access_token:libtokensetproc_shared",   # 不要用 libtoken_setproc（静态）
    ...
  ]
  ```
- **参考样板**：`foundation/distributedhardware/distributed_hardware_fwk/interfaces/inner_kits/test/demo/BUILD.gn` 是同子系统中同时用 `libnativetoken_shared` + `libtokensetproc_shared` 的 demo；新增可执行 demo 直接 copy 它的 external_deps 模板可少踩这个坑
- **何时复现**：`ohos_executable` 类型 + 自己调 `GetAccessTokenId` / `SetSelfTokenID` 的二进制；UT（`ohos_unittest`）通常通过 fuzz/gtest 框架间接拉 selinux_adapter，不一定触发
- **不要的"修复"**：不要给 demo 加 `selinux_adapter:librestorecon` 来"绕"——这只是补一个间接依赖，正确方向是让 native_token 走动态库版本，由动态库内部自带 selinux 解析

#### 5.2.8 板镜像必须与"编 dm so 用的 code base 同期"，否则 `__cfi_check_fail` SA 必崩

- **现象**：把云端编出来的 `libdevicemanagerservice.z.so` / `libdevicemanagerserviceimpl.z.so` 推到 RK3568 板、reboot 后 SA 拉起正常，但**调用任何走 BundleMgr 的 IPC 入口（如 `BindTarget`）时 SA 立即 `SIGABRT`**，回栈：
  ```
  Signal:SIGABRT(SI_TKILL)
  #02 __cfi_check_fail            libappexecfwk_core.z.so
  #03 __cfi_check                  libappexecfwk_core.z.so
  #04 DeviceManagerServiceImpl::GetBundleLable
  #05 DeviceManagerServiceImpl::GetBindCallerInfo
  #08 IpcCmdProcessBIND_TARGET
  ```
- **根因**：dm 编译时带 `sanitize.cfi = true` + `cfi_cross_dso = true`（见 `services/implementation/BUILD.gn`），跨 DSO 函数指针调用走 CFI 校验。云端代码与板镜像不同期 ⇒ 双方对 `IBundleMgr::GetApplicationInfoV9` 等签名生成的 CFI bitset 不一致 ⇒ 跳转时 type id 不匹配 ⇒ `__cfi_check_fail` 直接 `abort()`
- **判别**：
  - 看 `/data/log/faultlog/faultlogger/cppcrash-device_manager-*.log` 栈顶是否包含 `__cfi_check_fail` / `__cfi_check`
  - 是 → ABI 不对齐，本质是版本错配，不是 PR bug
  - 不是 → 才考虑 PR 自身逻辑问题
- **修复（按推荐度排序）**：
  1. **刷与云端 code base 同期的 OH 日构建到板上**（最干净）：从 [https://dcp.openharmony.cn/workbench/cicd/dailybuild/detail/component](https://dcp.openharmony.cn/workbench/cicd/dailybuild/detail/component) 下载对应日期的 RK3568 整机包，用 RKDevTool 刷机。例：云端 base 是 `openharmony_master_default_20260530...`，板上就刷 5/30 日构建
  2. **依赖 so 整套对齐**：除 dm so 外，把 `libappexecfwk_core.z.so` 等 CFI 校验失败涉及的依赖 so 一起从云端 `out/` 推到板上（容易漏，不推荐）
  3. **关掉 dm CFI 重新编**（仅作快速验证、不要提交）：临时把 `sanitize.cfi = false` 改进 dm BUILD.gn，重编推板。**绝对不要**把这种改动合到 PR
- **预防**：每次"云端编 → 推板验证"前先核对：板上 `param get const.ohos.version.security_patch` / 日构建时间戳 与 云端 code 的 `openharmony_master_default_<日期>...` 是否同日

#### 5.2.9 云上拉镜像：先云端打 tar.gz，再下载，再本地解压（不要直接 rsync 散文件）

> **凭据/物理环境隔离原则**：本节命令里的 `<HOST>` / `<PORT>` / `<USER>` / `<REMOTE_WORKSPACE>` / 任何密码都**不允许写入本知识库**或任何 docs / .codespec / 公共脚本。每次执行前由 agent 向当前会话用户询问，得到后只保留在会话上下文里（不落盘、不进 commit）。如遇到 PR/MR/issue 评论里贴了真实地址或密码，agent 必须当场提示作者撤回。

- **现象**：`out/rk3568/packages/phone/images/` 下散文件总量 ~2.4GB（system.img / vendor.img / userdata.img / sys_prod.img / ...），直接 `rsync` 走 ssh 单连接，带宽利用率低、易被 20 分钟 tool 超时截断、断点续传只能恢复"单个 img"，已传一半的大 img 经常要重来。
- **正确流程（节省带宽 + 可断点续传 + 不超时）**：
  1. **云端先打 tar.gz**（一次性顺序读盘、压缩比 ~70%）：
     ```bash
     # 示例占位，真实 <HOST>/<PORT>/<USER>/<REMOTE_WORKSPACE> 运行时问用户
     ssh -p <PORT> <USER>@<HOST> \
       "cd <REMOTE_WORKSPACE>/code/out/rk3568/packages/phone && \
        tar -czf rk_images.tar.gz images/"
     ```
     2.4GB 散文件 → 单个 ~700MB tar.gz。
  2. **rsync 单文件下载**（`--partial --inplace --progress` 保证断点续传）：
     ```bash
     rsync -av --partial --inplace --progress \
       --rsh="ssh -p <PORT> -o StrictHostKeyChecking=no" \
       <USER>@<HOST>:<REMOTE_WORKSPACE>/code/out/rk3568/packages/phone/rk_images.tar.gz \
       <LOCAL_DIR>/rk_images.tar.gz
     ```
     如必须用密码登录，优先 ssh-agent / `~/.ssh/config` 中配 key；万不得已用 `sshpass` 时，密码用 `SSHPASS` 环境变量读入，**绝不 `-p '<明文>'` 写进命令历史 / 脚本 / 日志**。
  3. **本地解压**：`tar -xzf <LOCAL_DIR>/rk_images.tar.gz -C <LOCAL_DIR>/`
- **反例**：散文件 rsync 跑 `system.img` 卡住 → 20 分钟 tool 超时 → 终端看似挂死、实际后台 rsync 还在跑/已挂但进程残留，需要 `pkill -f "rsync .*images"` 才能清干净。
- **额外好处**：tar.gz 单文件可直接给同事走 IM/网盘分发；散 img 包没 manifest 容易漏 `MiniLoaderAll.bin` 之类启动件。
- **检查清单**：
  - [ ] **本节命令里的 `<HOST>/<PORT>/<USER>/<REMOTE_WORKSPACE>/密码` 没有任何一项被写进 docs / .codespec / commit / PR 描述**
  - [ ] 云端打包前 `du -sh images/` 记录原始大小
  - [ ] 打包后 `ls -lh rk_images.tar.gz` 记录压缩后大小，写入验证报告（不带主机名）
  - [ ] 下载完 `tar -tzf rk_images.tar.gz | wc -l` 与云端 `ls images/ | wc -l` 对齐
  - [ ] 解压后核对 `MiniLoaderAll.bin / parameter.txt / system.img / vendor.img / userdata.img / sys_prod.img / chip_prod.img / resource.img / boot_linux.img / updater.img` 齐全

#### 5.2.10 验证可信关系（ACL）相关改动前，先清 DP RDB 历史数据再重测，避免旧记录污染结论

- **现象**：同一组板子反复跑 sink/source demo 验证 `access_control_table.extraData` 字段是否被新代码正确写入时，看到的可能是上一轮（旧 so 或旧逻辑）落下的 ACL 行，新 so 实际没写进去也会"看似 PASS"或"看似 FAIL"，掩盖真实行为。
- **典型坑**：
  - 旧 so 把 `ACL_LIFE_CYCLE_DAYS` 错写到 `accessee.extraData`，新 so 改写到 `access_control_table.extraData`，但 sink 板上旧那行 accessee 残留，新 so 因 `hasAcl=true` 走 update 分支只补 `access_control_table` 不会重置 accessee，结论混乱
  - hichain credential 也是持久化的，旧 credential 残留会让 source `BindTarget` 走"已认证 fast path"直接 OK，跳过新逻辑落 ACL，DB 里看不到新行
- **正确做法（推 so 验证前清干净）**：
  ```bash
  hdc -t <DEV> target mount
  hdc -t <DEV> shell "\
    rm -rf /data/service/el1/public/database/distributed_device_profile_service/acl_db/* ; \
    rm -rf /data/service/el1/public/database/distributed_device_profile_service/profile_data* ; \
    rm -rf /data/service/el1/public/database/distributed_device_profile_service/local_service_info_store* ; \
    rm -rf /data/service/el1/public/database/distributed_device_profile_service/kvdb/ ; \
    rm -rf /data/service/el1/public/database/distributed_device_manager_service/ ; \
    rm -rf /data/service/el1/public/hichainsdk/ ; \
    rm -f /data/local/tmp/issue2445_pin.txt /data/local/tmp/sink.log /data/local/tmp/source.log ; \
    sync"
  # 推新 so 后 reboot，DP / DM / hichain 进程拉起时各自重建空库
  hdc -t <DEV> shell reboot
  ```
- **为什么可以直接删**：DP（`distributed_device_profile_service`）/ DM（`distributed_device_manager_service`）/ hichain（`hichainsdk`）的持久化目录都是各自服务的私有 SQLite/KV 库，启动时检测到目录缺失/空表会自动重建 schema 和空表，**对运行时没有破坏性**。本质等同"业务侧 factory reset"。
- **判定 ACL 是否"由本次 demo 新落"**：
  ```sh
  # 删库 + reboot 后，库里应当只有空 schema、零行
  python3 -c "import pysqlite3; c=pysqlite3.connect('/tmp/dp_rdb_sink.db'); \
    print('rows:', c.execute('SELECT COUNT(*) FROM access_control_table').fetchone()[0])"
  # 跑完一轮 demo 再查，看到的每一行都来自本次 bind，能直接对比代码改动效果
  ```
- **不要清的内容**：板子的 `/data/misc_ce/<userId>/`、`/data/app/`、`/data/accounts/` 不要乱删，会破坏系统账户/应用沙箱，导致 dm SA 起不来。
- **检查清单**：
  - [ ] 改 ACL 写入/读取相关代码后，验证前先按上面命令清两板的 DP / DM / hichain 数据
  - [ ] reboot 后等 `bootevent.boot.completed=true` 再跑 demo
  - [ ] 跑前 `sqlite3` 看 `access_control_table` 行数应为 0
  - [ ] 跑后再查 DB，看到的所有 ACL 行均为本轮产物，可直接断言

#### 5.2.11 多用例连续跑：用例之间快速重置可信关系，不需要 reboot 也不需要清 hichain

- **场景**：同一对板子上要跑 N 个 ACL 行为用例（例如 `ACL_LIFE_CYCLE_DAYS = 1095 / -1 / 1 / 3650` 各跑一次），每个用例都要在"干净环境"下验证 `access_control_table.extraData` 的写入结果。逐个 reboot 太慢（每次都要重新 `usbipd attach`），全程跳过清理又会让上一轮 ACL 行污染下一轮（DM 看 `hasAcl=true` 走 update 分支，新行不写入）。
- **正确做法（轻量 reset，无需 reboot）**：每个用例跑完后，**只删 DP 的 ACL 数据库目录，再 kill DM 与 DP SA 让它们自动拉起**，hichain credential 不用动：
  ```bash
  # 在两板上各跑一次
  hdc -t <DEV> shell "\
    killall CollaborationFwk_source CollaborationFwk_sink 2>/dev/null; \
    kill -9 \$(pidof device_manager) \$(pidof deviceprofile) 2>/dev/null; \
    sleep 1; \
    rm -rf /data/service/el1/public/database/distributed_device_profile_service/{acl_db,profile_data*,local_service_info_store*,kvdb}/ 2>/dev/null"
  # 触发一次 DM/DP lazy load（任何 DM IPC 调用都行）
  hdc -t <DEV> shell "/data/local/tmp/dm_issue2445_p2_probe > /dev/null"
  hdc -t <DEV> shell "power-shell wakeup; power-shell setmode 602"
  ```
- **为什么不用清 hichain credential**：DM 判定"可信关系是否存在"**只看 DP `access_control_table`，不直接查 hichain credential**。DP ACL 被清空后，DM 视为"全新关系"，下一次 bind 走完整流程并重新落 ACL。hichain credential 的残留只影响 hichain 内部 session key 复用，不会让 DM 跳过 ACL 写入。这与 §5.2.10 中"首次验证或修了 DM ACL 代码后整库清"的场景不同——后者要确保**所有持久层**都从零开始排除历史影响，前者只需让"ACL 落盘行为"可复测。
- **为什么不用 reboot**：DM SA / DP SA 是 `device_manager.cfg` / `device_profile.cfg` 注册的按需 SA，kill 后下一次 IPC 调用即被 SAMgr 自动拉起，重建空 schema 与 case 1 reboot 等效。reboot 在 WSL 下还会断 USB，需要人工 `usbipd attach`，连续跑用例时成本太高。
- **DB 拉取时机的强约束**：用例跑完后**拉 `dp_rdb.db` 前不要 kill DP SA**——DP 写入走 WAL，DP 进程在世时 WAL pages 才完整可读；提前 kill DP 可能丢失未 checkpoint 的 WAL 数据，导致 `access_control_table` 查到 0 行的假阴性。正确顺序：
  1. sink/source bind 完成（source 拿到 `bindResult:0`）
  2. `sleep 3` 让 DP 把数据写完 WAL
  3. `hdc file recv` 拉 `dp_rdb.db` + `dp_rdb.db-wal` + `dp_rdb.db-shm`（**全套**，python sqlite3 自动合并 WAL）
  4. 本地 `python3 -c "import sqlite3; ..."` 解析
  5. 验证完了，再进入"清 DP DB + kill DM/DP SA"做下一个用例
- **多用例运行模板**（伪代码）：
  ```bash
  for case in case1 case2 case3 ...; do
    reset_env $DEV_A $DEV_B            # kill demo / DM / DP + rm DP db
    sink_run --life-cycle-days $val    # 在 sink 板跑 sink demo
    pin_ferry                          # PC 端转移 pin 文件 sink -> source
    source_run --life-cycle-days $val  # source 板跑 source demo
    sleep 3                            # 让 DP 写完 WAL
    pull_dp_rdb_and_verify             # 全套拉 db/-wal/-shm 后 python 解析
  done
  ```
- **检查清单**：
  - [ ] 每个用例之间只清 `/data/service/el1/public/database/distributed_device_profile_service/` 下的 ACL/profile/local_service_info/kvdb 目录
  - [ ] hichain credential 与 `/data/service/el1/public/database/distributed_device_manager_service/` 可不动（DM 不依赖）
  - [ ] kill DM SA + DP SA 后用 `dm_issue2445_p2_probe` 或任意 DM IPC 触发 lazy load
  - [ ] 拉 DP RDB 时 DP SA 必须在运行，且要拉全套（.db + -wal + -shm）
  - [ ] 用例间不需要 reboot，节省 WSL 重新 attach 时间

### 5.3 验证流程闭环 checklist

每次跑 DM 相关验证（UT / 独立归档 demo / xdevice 端到端）前过一遍：

- [ ] PR 改动的源文件已识别完整
- [ ] 对应 .so 已用 `hb build -T <target>` 重编
- [ ] 所有目标板已 `hdc target mount` + 推完 .so + `sync` + reboot + `wait-for-device` + 等 30s BOOT_COMPLETED
- [ ] 测试二进制已推 `/data/local/tmp/` 且 `chmod 755`
- [ ] 跑测试前先确认 `ps -ef | grep device_manager` 看到 SA 在，且 `/data/log/faultlog/` 没有刚生成的 DM cppcrash
- [ ] 验证日志里出现 PR 引入的关键字（如 `ACL_LIFE_CYCLE_DAYS`），证明 .so 确实生效
- [ ] **若开发主机是 WSL2**：板子 reboot 后 USB 设备会断链，必须由 Windows 侧 `usbipd attach` 重新挂载到 WSL（见 §5.4）

### 5.4 WSL2 + usbipd：板子 reboot 后必须重新 attach

WSL2 通过 `usbipd-win` 把 Windows 主机上的 USB 设备透传到 WSL 用户态。RK3568 等开发板 `reboot` 后 USB 链路重新枚举，**Windows 侧自动收回设备所有权**，WSL 内 `hdc list targets` 会返回 `[Empty]` 直到重新 attach。

**症状**

```bash
hdc list targets
# [Empty]

# Windows 侧 (在 WSL 内调用 .exe)
'/mnt/c/Program Files/usbipd-win/usbipd.exe' list
# BUSID  VID:PID    DEVICE         STATE
# 1-1    2207:5000  "HDC Device"   Shared        <- 已收回，未 attach
# 2-3    2207:5000  "HDC Device"   Shared        <- 同上
```

**根因**

- `usbipd` 的 `Shared` 状态表示设备**允许**被 WSL attach，但**当前未** attach
- `reboot` → USB 重新枚举 → `usbipd` 检测到新设备 → 自动 detach → 回到 `Shared` 状态
- 在 WSL 内调 `hdc list targets` 看不到设备直到下一次 attach

**恢复流程**

1. 查 Windows 侧目前看到几块板子：

   ```bash
   '/mnt/c/Program Files/usbipd-win/usbipd.exe' list | grep -E "HDC|2207"
   ```

2. 对每个 `Shared` 状态的 HDC busid 重新 attach：

   ```bash
   '/mnt/c/Program Files/usbipd-win/usbipd.exe' attach --wsl --busid 1-1
   '/mnt/c/Program Files/usbipd-win/usbipd.exe' attach --wsl --busid 2-3
   ```

3. 等几秒让 `hdcd` 在板子侧重启完，确认两板可见：

   ```bash
   sleep 5 && hdc list targets
   # 150100424a5444345209d94ebed7b900
   # 450100445634303332011a0f0a2eba00
   ```

4. 若仍 `[Empty]`，重启 hdc 守护：

   ```bash
   hdc kill && sleep 2 && hdc start -r && sleep 5 && hdc list targets
   ```

**自动化建议**

把 attach 步骤封装成脚本（如 `~/.opencode/skills/distributed-ai-auto-test/scripts/attach-boards.sh`），在 xdevice runner 之前先调一次，避免每次 reboot 后手忙脚乱：

```bash
#!/bin/bash
# attach-boards.sh: re-attach all "Shared" HDC devices to WSL
USBIPD='/mnt/c/Program Files/usbipd-win/usbipd.exe'
"$USBIPD" list | awk '/2207:5000.*Shared$/ {print $1}' | while read busid; do
    "$USBIPD" attach --wsl --busid "$busid"
done
sleep 5
hdc list targets
```

**常见坑**

- **只 attach 一块板子**：双板验证场景必须确认 `hdc list targets` 返回两个 SN，否则后续 xdevice 自动选板会拿错
- **`Persisted` vs `Shared`**：第一次用某块板时先在 Windows 管理员 PowerShell 里 `usbipd bind --busid <X>`，让设备进入 `Shared` 列表，否则 attach 会报权限错
- **板子 BOOT_COMPLETED 之前 attach**：可能 attach 到内核 USB 阶段，hdc 仍连不上；建议等 ~30 秒板子完全起来再 attach；或先 attach、再 `hdc wait-for-device`
- **WSL Distro 名不对**：`usbipd attach --wsl` 默认挂到第一个 WSL2 distro；用 `--wsl <DistroName>` 显式指定避免误挂

---

## 6. 故障排查流程

### 6.1 认证失败排查

认证失败是最常见的问题之一，可能涉及 HiChain、SoftBus、权限等多个方面。

#### 6.1.1 排查步骤

**步骤 1：检查错误码**

```bash
# 查看认证错误日志
hdc shell hilog -t DHDM -b E | grep "auth"

# 查找常见错误码
# -20018: 不支持的认证类型
# -20019: 认证业务繁忙
# -20021: 对端拒绝认证
# -20022: 认证被拒绝
# -20026: 创建群组失败
# -20053: 认证码错误
# -20056: PIN 码显示超时
# 96929762: 打开会话失败
# 96929765: 认证失败
# 96929774: 添加群组失败
```

**步骤 2：检查 HiChain 状态**

```bash
# 检查 HiChain 服务状态
hdc shell ps -ef | grep hichain

# 查看 HiChain 日志
hdc shell hilog -t HiChain | grep -i error

# 检查群组信息
hdc shell "hiChain -l"
```

**步骤 3：检查 SoftBus 会话**

```bash
# 检查 SoftBus 会话状态
hdc shell "softbus_client -s"

# 查看 SoftBus 日志
hdc shell hilog -t Softbus | grep -i session

# 检查网络连接
hdc shell "netstat -an | grep 5000"
```

**步骤 4：检查权限和 Token**

```bash
# 检查应用权限
hdc shell "bm dump -n <PackageName> | grep permission"

# 检查 AccessToken
hdc shell "acm dump"

# 查看 Token 信息
hdc shell "token dump"
```

**步骤 5：检查设备时间同步**

```bash
# 检查系统时间
hdc shell date

# 检查时区设置
hdc shell getprop persist.sys.timezone

# 确保两设备时间差不超过 5 分钟
```

#### 6.1.2 常见问题与解决方案

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| **错误码 -20018** | 不支持的认证类型 | 检查 `authType` 参数，确保两端支持的认证类型匹配 |
| **错误码 -20026** | HiChain 创建群组失败 | 检查 HiChain 服务状态，查看 HiChain 日志 |
| **错误码 96929762** | 打开会话失败 | 检查 SoftBus 连接状态，确认网络正常 |
| **错误码 96929765** | 认证失败 | 检查认证流程、确认码、PIN 码是否正确 |
| **认证超时** | 网络延迟或对端无响应 | 检查网络连接，增加超时时间配置 |
| **认证码错误** | 用户输入错误 | 重试认证，确保确认码一致 |
| **重复认证** | 设备已认证 | 先删除信任关系再重新认证 |

---

### 6.2 设备发现不到

设备发现失败通常与 SoftBus、网络配置、发布订阅有关。

#### 6.2.1 排查步骤

**步骤 1：检查 SoftBus 服务**

```bash
# 检查 SoftBus 进程
hdc shell ps -ef | grep softbus

# 查看 SoftBus 日志
hdc shell hilog -t Softbus | grep -i discover

# 检查 LNN（本地网络）状态
hdc shell "softbus_client -l"
```

**步骤 2：检查设备发布状态**

```bash
# 检查设备是否发布
hdc shell "hidumper -s 3511 -a '--publish-info'"

# 查看发布参数
hdc shell "hidumper -s 3511 -a '--publish-params'"
```

**步骤 3：检查发现参数**

```bash
# 查看发现订阅信息
hdc shell "hidumper -s 3511 -a '--subscribe-info'"

# 检查发现能力匹配
hdc shell hilog -t DH_DM_DISCOVER | grep -i capability
```

**步骤 4：检查网络连接**

```bash
# 检查 Wi-Fi 状态
hdc shell "ifconfig wlan0"

# 检查蓝牙状态
hdc shell "hciconfig"

# 检查网络邻居
hdc shell "ip neigh show"
```

**步骤 5：检查配置参数**

```bash
# 检查发现介质配置
hdc shell getprop hw.discover.medium

# 检查发现模式
hdc shell getprop hw.discover.mode
```

#### 6.2.2 常见问题与解决方案

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| **发现不到设备** | SoftBus 未启动 | 重启 SoftBus 服务 |
| **发现超时** | 发现时间过短 | 增加发现超时时间（默认 60 秒） |
| **发现不到特定设备** | 能力不匹配 | 检查 `capability` 参数，确保两端能力匹配 |
| **BLE 发现失败** | 蓝牙未开启或权限不足 | 开启蓝牙，检查蓝牙权限 |
| **WiFi 发现失败** | 不在同一网络或 WiFi 未连接 | 确保两设备在同一局域网 |
| **发布失败** | 发布参数错误 | 检查发布模式和介质参数 |
| **订阅失败** | 订阅参数不匹配 | 检查订阅过滤条件 |

---

### 6.3 设备反复上下线

设备频繁上下线通常与网络不稳定、ACL 老化、状态管理有关。

#### 6.3.1 排查步骤

**步骤 1：检查网络稳定性**

```bash
# 查看设备上下线日志
hdc shell hilog -t DH_DM_STATE | grep -i "online\|offline"

# 检查网络连接状态
hdc shell "ping <peer_ip> -c 100"

# 查看 SoftBus 连接状态
hdc shell "softbus_client -c"
```

**步骤 2：检查 ACL 老化配置**

```bash
# 查看 ACL 老化版本支持
hdc shell getprop hw.dm.acl.version

# 检查 ACL 状态
hdc shell "hidumper -s 3511 -a '--acl-status'"

# 查看 ACL 老化日志
hdc shell hilog -t DH_DM_ACL | grep -i aging
```

**步骤 3：检查设备状态机**

```bash
# 查看设备状态转换日志
hdc shell hilog -t DH_DM_STATE | grep -i "state.*change"

# 查看状态机异常
hdc shell hilog -t DH_DM_STATE -b E
```

**步骤 4：检查心跳和超时**

```bash
# 查看心跳日志
hdc shell hilog -t DHDM | grep -i heartbeat

# 检查会话超时配置
hdc shell getprop hw.session.timeout
```

#### 6.3.2 常见问题与解决方案

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| **频繁上下线** | 网络不稳定 | 检查网络连接，切换到更稳定的网络 |
| **ACL 老化导致下线** | ACL 过期 | 重新认证，更新 ACL |
| **心跳超时** | 会话超时时间过短 | 增加会话超时配置 |
| **状态机死锁** | 状态转换异常 | 重启 DeviceManager 服务 |
| **多设备冲突** | 同一网络多设备 ID 冲突 | 检查设备 ID，确保唯一性 |

---

### 6.4 ACL 老化异常

ACL（访问控制列表）老化是版本 5.1.0 引入的安全特性，但可能导致设备掉线。

#### 6.4.1 排查步骤

**步骤 1：检查版本兼容性**

```bash
# 查看 DM 版本
hdc shell getprop hw.dm.version

# 检查是否支持 ACL 老化
hdc shell getprop hw.dm.acl.aging.enable

# 查看支持的 ACL 老化版本
hdc shell "hidumper -s 3511 -a '--acl-version'"
```

**步骤 2：检查 ACL 状态**

```bash
# 查看 ACL 信息
hdc shell "hidumper -s 3511 -a '--acl-info -networkId xxx'"

# 查看 ACL 过期时间
hdc shell hilog -t DH_DM_ACL | grep -i expire

# 检查 ACL 更新日志
hdc shell hilog -t DH_DM_ACL | grep -i refresh
```

**步骤 3：检查 HiChain 凭据**

```bash
# 查看 HiChain 凭据
hdc shell "hiChain -c"

# 检查凭据过期时间
hdc shell hilog -t HiChain | grep -i credential
```

#### 6.4.2 常见问题与解决方案

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| **ACL 过期掉线** | ACL 老化时间到期 | 重新认证，更新 ACL |
| **版本不兼容** | 对端版本不支持 ACL 老化 | 升级对端版本到 5.1.0+ |
| **ACL 更新失败** | HiChain 凭据更新失败 | 检查 HiChain 服务，手动更新凭据 |
| **ACL 权限不足** | 权限配置错误 | 检查权限配置，重新授权 |

---

### 6.5 多用户切换后设备不可见

多用户场景下，设备可见性与用户绑定和权限管理有关。

#### 6.5.1 排查步骤

**步骤 1：检查用户切换事件**

```bash
# 查看用户切换日志
hdc shell hilog -t DHDM | grep -i "user.*switch"

# 查看当前用户 ID
hdc shell "bm dump --user | grep current"

# 检查用户切换事件
hdc shell hidumper -s 1202 -a "-domain DISTDM -event COMMON_EVENT_USER_SWITCHED"
```

**步骤 2：检查设备用户绑定**

```bash
# 查看设备用户绑定信息
hdc shell "hidumper -s 3511 -a '--user-binding'"

# 查看特定用户的设备
hdc shell "hidumper -s 3511 -a '--user-device -userId xxx'"
```

**步骤 3：检查权限和 Token**

```bash
# 检查用户 Token
hdc shell "token dump --user <userId>"

# 查看用户权限
hdc shell "acm dump --user <userId>"
```

#### 6.5.2 常见问题与解决方案

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| **切换后设备不可见** | 设备绑定到其他用户 | 切换回原用户或重新绑定设备 |
| **权限丢失** | 用户切换导致权限变化 | 重新请求权限 |
| **设备列表为空** | 用户切换时设备信息未同步 | 重启 DeviceManager 服务 |
| **多用户认证失败** | 多用户认证未配置正确 | 检查多用户认证配置 |

---

### 6.6 绑定/解绑失败

绑定和解绑操作涉及多个模块，失败时需要系统排查。

#### 6.6.1 排查步骤

**步骤 1：检查绑定流程**

```bash
# 查看绑定日志
hdc shell hilog -t DH_DM_AUTH | grep -i "bind"

# 查看绑定错误码
hdc shell hilog -t DHDM -b E | grep -i "bind.*failed"
```

**步骤 2：检查解绑流程**

```bash
# 查看解绑日志
hdc shell hilog -t DHDM | grep -i "unbind\|delete.*trust"

# 查看解绑错误码
hdc shell hilog -t DHDM -b E | grep -i "unbind.*failed"
```

**步骤 3：检查 DP（设备档案）状态**

```bash
# 查看 DP 服务状态
hdc shell ps -ef | grep distributedprofile

# 查看 DP 日志
hdc shell hilog -t DISTRIBUTEDPROFILE | grep -i error
```

#### 6.6.2 常见问题与解决方案

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| **绑定超时** | 网络延迟或对端无响应 | 检查网络，增加超时时间 |
| **绑定被拒绝** | 用户取消或对端拒绝 | 重新发起绑定，确保用户确认 |
| **解绑失败** | 设备不存在或权限不足 | 检查设备状态，确认权限 |
| **DP 同步失败** | DP 服务异常 | 重启 DP 服务，检查 DP 日志 |

---

## 7. 依赖模块排查

DeviceManager 依赖多个基础服务，需要系统排查依赖模块状态。

### 7.1 SoftBus 状态检查

SoftBus 是设备间通信的基础设施，状态异常会导致发现、认证等功能失败。

#### 7.1.1 SoftBus 服务状态

```bash
# 检查 SoftBus 进程
hdc shell ps -ef | grep softbus

# 查看 SoftBus 服务列表
hdc shell "softbus_client -s"

# 检查 SoftBus 日志
hdc shell hilog -t Softbus | grep -i error
```

#### 7.1.2 SoftBus 会话状态

```bash
# 查看会话列表
hdc shell "softbus_client -o"

# 查看特定会话
hdc shell "softbus_client -i <sessionId>"

# 查看会话详情
hdc shell "softbus_client -d <sessionId>"
```

#### 7.1.3 SoftBus LNN 状态

```bash
# 查看 LNN 信息
hdc shell "softbus_client -l"

# 查看在线设备
hdc shell "softbus_client -n"

# 查看 LNN 能力
hdc shell "softbus_client -c"
```

#### 7.1.4 SoftBus 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| **SoftBus 未启动** | 服务启动失败 | 重启 SoftBus 服务 |
| **会话打开失败** | 网络异常或对端不可达 | 检查网络，确保对端在线 |
| **LNN 离线** | 设备网络异常 | 检查网络连接，重启 LNN |
| **SoftBus 崩溃** | 内存不足或异常 | 检查系统日志，重启设备 |

---

### 7.2 DP 状态检查

DP（DeviceProfile）负责设备信息和信任关系存储。

#### 7.2.1 DP 服务状态

```bash
# 检查 DP 进程
hdc shell ps -ef | grep distributedprofile

# 查看 DP 服务
hdc shell "sa list | grep distributedprofile"

# 检查 DP 日志
hdc shell hilog -t DISTRIBUTEDPROFILE | grep -i error
```

#### 7.2.2 DP 数据检查

```bash
# 查看设备档案
hdc shell "distributedprofile_client -d"

# 查看信任关系
hdc shell "distributedprofile_client -t"

# 查看 DP 同步状态
hdc shell "distributedprofile_client -s"
```

#### 7.2.3 DP 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| **DP 服务未启动** | 服务启动失败 | 重启 DP 服务 |
| **设备信息未同步** | 网络异常或 DP 异常 | 检查网络，重启 DP 服务 |
| **信任关系丢失** | DP 数据损坏 | 恢复 DP 数据或重新认证 |
| **同步超时** | 网络延迟 | 检查网络，增加超时时间 |

---

### 7.3 HiChain 状态检查

HiChain 负责设备认证和凭据管理。

#### 7.3.1 HiChain 服务状态

```bash
# 检查 HiChain 进程
hdc shell ps -ef | grep hichain

# 查看 HiChain 服务
hdc shell "sa list | grep hichain"

# 检查 HiChain 日志
hdc shell hilog -t HiChain | grep -i error
```

#### 7.3.2 HiChain 群组管理

```bash
# 查看群组列表
hdc shell "hiChain -g"

# 查看群组成员
hdc shell "hiChain -m <groupId>"

# 查看凭据信息
hdc shell "hiChain -c"
```

#### 7.3.3 HiChain 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| **HiChain 未启动** | 服务启动失败 | 重启 HiChain 服务 |
| **群组创建失败** | 参数错误或权限不足 | 检查群组参数，确认权限 |
| **凭据过期** | 凭据超时 | 重新认证，更新凭据 |
| **添加成员失败** | 对端设备异常 | 检查对端状态，重试添加 |

---

## 8. 常用调试工具

### 8.1 HiLogViewer

HiLogViewer 是 OpenHarmony 提供的日志查看工具，支持实时查看和过滤日志。

**功能特性**：
- 实时日志流式查看
- 多标签过滤
- 日志级别过滤
- 关键字搜索
- 日志导出

**使用方法**：
```bash
# 启动 HiLogViewer
hdc shell hilog_viewer

# 或者使用命令行过滤
hdc shell hilog -t DHDM -b D | grep "authentication"
```

### 8.2 HiDumper

HiDumper 是系统信息转储工具，用于获取系统状态和服务信息。

**DeviceManager 相关命令**：
```bash
# 查看设备列表
hdc shell "hidumper -s 3511 -a '--device-list'"

# 查看服务状态
hdc shell "hidumper -s 3511 -a '--service-status'"

# 查看认证状态
hdc shell "hidumper -s 3511 -a '--auth-status'"

# 查看发现状态
hdc shell "hidumper -s 3511 -a '--discover-status'"
```

### 8.3 HDC Shell

HDC Shell 提供设备命令行访问，类似于 Android 的 adb shell。

**常用命令**：
```bash
# 进入 Shell
hdc shell

# 查看进程
ps -ef | grep devicemanager

# 查看网络状态
netstat -an

# 查看系统属性
getprop

# 查看系统时间
date
```

### 8.4 性能分析工具

#### 8.4.1 CPU 性能分析

```bash
# 查看 CPU 使用率
hdc shell top -n 1 | grep devicemanager

# 使用 perf 工具分析
hdc shell perf top -p <pid>
```

#### 8.4.2 内存分析

```bash
# 查看内存使用
hdc shell "cat /proc/<pid>/status"

# 使用他命令查看内存映射
hdc shell "pmap <pid>"
```

#### 8.4.3 网络分析

```bash
# 抓包分析
hdc shell "tcpdump -i any -w /data/capture.pcap"

# 查看网络连接
hdc shell "netstat -an"
```

### 8.5 故障注入工具

用于测试系统异常情况的处理能力。

#### 8.5.1 网络故障注入

```bash
# 模拟网络延迟
hdc shell "tc qdisc add dev wlan0 root netem delay 100ms"

# 模拟丢包
hdc shell "tc qdisc add dev wlan0 root netem loss 10%"
```

#### 8.5.2 服务故障注入

```bash
# 停止 SoftBus
hdc shell "killall softbus_server"

# 停止 HiChain
hdc shell "killall hichain"
```

---

## 9. 调试最佳实践

### 9.1 日志记录规范

1. **关键流程必须记录**：认证、发现、状态变化等关键节点必须记录日志
2. **错误信息详细**：错误日志应包含错误码、错误原因、上下文信息
3. **敏感信息脱敏**：UDID、Token 等敏感信息必须脱敏
4. **日志级别合理**：DEBUG 信息仅在开发阶段使用，发布版本使用 INFO 及以上

### 9.2 故障定位流程

1. **复现问题**：明确问题复现步骤
2. **收集日志**：使用 hilog 收集相关日志
3. **分析错误码**：根据错误码定位问题模块
4. **检查依赖**：检查 SoftBus、HiChain 等依赖服务状态
5. **定位根因**：结合日志和代码定位根本原因
6. **验证修复**：修复后验证问题是否解决

### 9.3 调试技巧

1. **二分法定位**：通过开关不同模块逐步缩小问题范围
2. **对比法**：对比正常设备和异常设备的日志差异
3. **增量调试**：逐步添加调试信息，精确定位问题点
4. **压力测试**：通过高频操作复现偶现问题
5. **版本回退**：对比不同版本的行为差异

---

## 10. 附录

### 10.1 常用错误码速查表

| 错误码 | 说明 | 常见原因 |
|--------|------|----------|
| **0** | 成功 | - |
| **-20001** | 超时 | 网络延迟、对端无响应 |
| **-20002** | 对端确认超时 | 用户未确认、对端异常 |
| **-20018** | 不支持的认证类型 | 认证类型不匹配 |
| **-20019** | 认证业务繁忙 | 并发认证过多 |
| **-20021** | 对端拒绝认证 | 用户拒绝 |
| **-20022** | 认证被拒绝 | 权限不足、参数错误 |
| **-20026** | 创建群组失败 | HiChain 异常 |
| **-20037** | 用户取消绑定 | 用户主动取消 |
| **-20042** | 用户取消绑定错误 | 绑定流程异常 |
| **-20053** | 认证码错误 | 用户输入错误 |
| **-20056** | PIN 码显示超时 | 超时时间过短 |
| **-20058** | 重复删除设备 | 设备已删除 |
| **-20059** | 版本不兼容 | 版本不匹配 |
| **96928545** | 设备不支持 | 功能不支持 |
| **96929744** | 失败 | 通用错误 |
| **96929746** | 未初始化 | 服务未初始化 |
| **96929747** | 初始化失败 | 服务启动失败 |
| **96929748** | 空指针 | 参数为空 |
| **96929749** | 参数无效 | 参数错误 |
| **96929750** | 无权限 | 权限不足 |
| **96929751** | 内存分配失败 | 内存不足 |
| **96929752** | 发现失败 | SoftBus 异常 |
| **96929753** | 键已存在 | 重复操作 |
| **96929754** | IPC 写失败 | IPC 异常 |
| **96929755** | IPC 拷贝失败 | IPC 异常 |
| **96929756** | IPC 发送请求失败 | IPC 异常 |
| **96929757** | 不支持的 IPC 命令 | 版本不匹配 |
| **96929758** | IPC 响应失败 | IPC 异常 |
| **96929759** | 重复发现 | 发现已启动 |
| **96929762** | 打开会话失败 | SoftBus 异常 |
| **96929765** | 认证失败 | HiChain 异常 |
| **96929766** | 认证未启动 | 认证流程错误 |
| **96929767** | 认证消息不完整 | 消息异常 |
| **96929769** | IPC 读失败 | IPC 异常 |
| **96929770** | 加密失败 | 加密模块异常 |
| **96929771** | 发布失败 | SoftBus 异常 |
| **96929772** | 重复发布 | 发布已启动 |
| **96929773** | 停止发现失败 | 发现状态异常 |
| **96929774** | 添加群组失败 | HiChain 异常 |
| **96929775** | 适配器未初始化 | 适配器未启动 |
| **96929776** | 不支持的方法 | 版本不匹配 |
| **96929777** | 通用绑定失败 | 绑定流程异常 |
| **96929778** | 绑定参数无效 | 参数错误 |
| **96929779** | PIN 码错误 | 用户输入错误 |
| **96929781** | 绑定超时 | 网络延迟 |
| **96929782** | DP 错误 | DP 异常 |
| **96929783** | HiChain 错误 | HiChain 异常 |
| **96929784** | SoftBus 错误 | SoftBus 异常 |
| **96929786** | 停止发布 LNN 失败 | LNN 状态异常 |
| **96929787** | 刷新 LNN 失败 | LNN 状态异常 |
| **96929788** | 停止刷新 LNN 失败 | LNN 状态异常 |
| **96929789** | 启动广告失败 | SoftBus 异常 |
| **96929790** | 停止广告失败 | SoftBus 异常 |
| **96929791** | 启用发现监听器失败 | SoftBus 异常 |
| **96929792** | 禁用发现监听器失败 | SoftBus 异常 |
| **96929793** | 启动发现失败 | SoftBus 异常 |
| **96929794** | 停止发现失败 | SoftBus 异常 |
| **96929795** | SoftBus 服务未初始化 | SoftBus 未启动 |
| **96929797** | 元类型无效 | 参数错误 |
| **96929798** | 加载自定义元节点失败 | 元节点异常 |
| **96929800** | SoftBus 发布服务 | SoftBus 异常 |
| **96929802** | 对端不支持绑定 | 版本不匹配 |
| **96929803** | HiChain 凭据请求失败 | HiChain 异常 |
| **96929804** | HiChain 凭据检查失败 | HiChain 异常 |
| **96929805** | HiChain 凭据导入失败 | HiChain 异常 |
| **96929806** | HiChain 凭据删除失败 | HiChain 异常 |
| **96929807** | HiChain 注销回调失败 | HiChain 异常 |
| **96929808** | HiChain 获取注册信息失败 | HiChain 异常 |
| **96929809** | HiChain 凭据已存在 | 凭据重复 |

### 10.2 相关文档

- 《DeviceManager 架构设计文档》
- 《DeviceManager API 参考手册》
- 《SoftBus 开发指南》
- 《HiChain 开发指南》
- 《OpenHarmony HiLog 使用指南》
- 《OpenHarmony HiSysEvent 开发指南》

### 10.3 联系方式

如有问题或建议，请联系：
- 邮件列表：devicemanager@opensource.huawei.com
- Issue 跟踪：https://gitee.com/openharmony/distributedhardware_device_manager/issues

---

**文档结束**
