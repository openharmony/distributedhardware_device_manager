# DeviceManager 模块领域知识库

**版本**：v2.0
**更新日期**：2026-05-20
**代码仓**：https://gitee.com/openharmony/distributedhardware_device_manager
**依赖模块**：软总线(DSoftBus)、设备档案(DP)、HiChain认证

## 知识库总览

本文档是 DeviceManager 模块的**唯一总入口**，采用总分结构组织所有领域知识。所有详细文档统一存放在 `docs/` 目录下，通过本页面导航访问。

## 模块核心定位

DeviceManager 是 OpenHarmony 分布式设备管理栈的**业务编排层**，负责账号无关的分布式设备认证、组网管理和可信关系维护。

```
┌─────────────────────────────────────────────────────┐
│                    应用层                             │
│    (系统应用通过 JS/NDK/Cangjie API 调用)            │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│               DM — 业务编排层                         │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌─────────┐ │
│  │ 发现管理  │ │ 认证编排  │ │ 状态管理  │ │ 用户管理 │ │
│  └────┬─────┘ └────┬─────┘ └────┬─────┘ └────┬────┘ │
│       │            │            │             │      │
│  ┌────▼────────────▼────────────▼─────────────▼────┐ │
│  │         ACL / SessionKey / 凭证组 管理           │ │
│  └────┬────────────┬───────────────────────────────┘ │
└───────┼────────────┼─────────────────────────────────┘
        │            │
┌───────▼─────┐  ┌───▼──────────┐
│  SoftBus    │  │   DP         │
│ 通信基础设施│  │ 数据存储层   │
└──────┬──────┘  └───┬──────────┘
       │              │
┌──────▼──────────────▼───────────────────────────────┐
│              HiChain / 系统基础设施                    │
└─────────────────────────────────────────────────────┘
```

- **向上**：为上层应用提供统一的设备发现、认证、绑定和解绑能力
- **向下**：封装软总线、设备档案和 HiChain 的底层通信与数据存储细节
- **横向**：与 DP、软总线共同构成完整的分布式设备管理全栈，三者存在深度耦合关系

## 核心能力矩阵

| 能力域 | 具体功能 |
|--------|---------|
| 设备发现 | 周边设备发布与扫描、可信设备列表查询、设备过滤与排序 |
| 设备监听 | SA 启停感知、DM 死亡通知、设备上下线监听、依赖 SA 就绪监听 |
| 信任管理 | HiChain 凭证组管理、ACL 全生命周期、SessionKey 管理、三阶段老化 |
| 认证绑定 | 6 种认证方式（PIN/QR/NFC/免交互/导入码/超声波）、绑定/解绑协议 |
| 用户适配 | 多用户隔离、前后台切换、账号事件处理、用户级 ACL active/inactive |
| 权限管控 | TokenID 校验、调用方身份验证、黑名单、系统应用校验 |
| 设备上下线 | 物理/逻辑双层模型、ONLINE→READY→OFFLINE 状态机、离线超时与重连 |

## 场景速查表

| 我要做的改动 | 先读 |
|---|---|
| 理解 DM 整体架构和模块分层 | [01-architecture](docs/01-architecture.md) |
| 搞清 LNN、ACL、HiChain、凭证组等术语 | [02-core-concepts](docs/02-core-concepts.md) |
| 查看 DeviceInfo、AuthParam 等结构体定义 | [03-data-structures](docs/03-data-structures.md) |
| 改设备发布、发现或查询逻辑 | [设备发布、发现、查询](docs/04-workflows/01-device-publish-discovery-query.md) |
| 改 SA 监听、死亡通知、依赖就绪逻辑 | [设备监听体系](docs/04-workflows/02-device-listener.md) |
| 改 ACL、SessionKey、凭证组、三阶段老化 | [可信关系管理](docs/04-workflows/03-trust-management.md) |
| 改多用户、前后台、账号切换 | [用户事件处理](docs/04-workflows/04-user-event-handling.md) |
| 改权限校验、调用方身份验证 | [权限校验](docs/04-workflows/05-permission-check.md) |
| 改设备上下线、JoinLNN/LeaveLNN | [设备上下线](docs/04-workflows/06-device-online-offline.md) |
| 改绑定/解绑、认证状态机、PinHolder | [绑定/解绑](docs/04-workflows/07-bind-unbind.md) |
| 开发上层应用调用 DM 接口 | [对外 SA 接口](docs/05-interfaces/01-external-api.md) |
| 理解 DM 内部模块间调用关系 | [内部模块接口](docs/05-interfaces/02-internal-api.md) |
| 改 DM 与 SoftBus/DP/HiChain 的交互 | [依赖模块接口](docs/05-interfaces/03-dependency-api.md) |
| 排查线上问题 | [调试指南](docs/06-debugging.md) |

## 全局术语表

| 术语 | 全称 | 含义 |
|------|------|------|
| DM | DeviceManager | 分布式设备管理器，本模块 |
| DP | DeviceProfile / DeviceInfoManager | 设备档案管理，可信关系数据存储层 |
| SoftBus / DSoftBus | Distributed Soft Bus | 分布式软总线，通信基础设施 |
| HiChain | — | 分布式信任管理框架，负责凭证组与认证协议 |
| LNN | Local Node Networking | 本地节点网络，SoftBus 管理的逻辑网络 |
| ACL | Access Control List | 访问控制列表，定义设备间信任权限 |
| SessionKey | — | 设备间安全通信的会话密钥 |
| PinHolder | — | PIN 码持有者，管理 PIN 交换的生命周期 |
| SA | System Ability | OpenHarmony 系统能力，DM 注册为 SA |
| DDS | Distributed Data Service | 分布式数据服务，设备信息同步 |
| UDS | Unix Domain Socket | Unix 域套接字，IPC 传输通道 |
| UKCache | User Key Cache | 用户密钥缓存，ACL 老化过渡态存储 |
| BR | Basic Rate | 蓝牙基础速率 |
| BLE | Bluetooth Low Energy | 低功耗蓝牙 |
| NCM | Network Control Model | USB 网络控制模型 |

## 文档导航

### 基础入门

1. [整体架构设计](docs/01-architecture.md) — 模块分层结构、系统定位、与软总线和 DP 的耦合关系
2. [核心概念与术语](docs/02-core-concepts.md) — 关键术语定义与业务含义
3. [关键数据结构](docs/03-data-structures.md) — 核心数据模型与字段说明

### 业务流程

4. [设备发布、发现、查询](docs/04-workflows/01-device-publish-discovery-query.md) — 发布-发现对称模型与设备查询
5. [设备监听体系](docs/04-workflows/02-device-listener.md) — SA 生命周期、死亡通知、上下线监听、依赖 SA 就绪
6. [DM 可信关系管理](docs/04-workflows/03-trust-management.md) — ACL/SessionKey/凭证组全生命周期，含三阶段老化
7. [用户事件处理](docs/04-workflows/04-user-event-handling.md) — 多用户隔离、前后台切换、账号事件全链路
8. [权限校验](docs/04-workflows/05-permission-check.md) — 调用方身份验证与权限管控
9. [设备上下线](docs/04-workflows/06-device-online-offline.md) — 物理/逻辑双层模型与状态机
10. [绑定/解绑](docs/04-workflows/07-bind-unbind.md) — 完整协议流程、认证状态机、PinHolder

### 开发参考

11. [对外 SA 接口](docs/05-interfaces/01-external-api.md) — JS/NAPI、NDK C、Cangjie FFI 接口详细文档
12. [内部模块接口](docs/05-interfaces/02-internal-api.md) — DM 各 Manager 间调用关系与接口契约
13. [依赖模块接口](docs/05-interfaces/03-dependency-api.md) — DM↔SoftBus、DM↔DP、DM↔HiChain 的 API 清单与约束

### 运维与问题定位

14. [调试与问题定位指南](docs/06-debugging.md) — 日志配置、调试命令、故障排查流程
15. [历史设计决策记录](docs/07-design-decisions.md) — ADR 格式记录重大架构决策
16. [常见问题解答](docs/08-faq.md) — 按场景分类的 FAQ
17. [技术演进路线图](docs/09-roadmap.md) — 技术演进方向与待改进项

## 贡献规范

1. 所有功能变更必须同步更新对应文档
2. 文档与代码同仓提交，版本保持一致
3. 核心流程文档必须包含 Mermaid 流程图或架构图
4. 接口文档必须包含：接口名、级别、涉及权限、功能描述、使用场景、参数详解
5. PR 标题格式：`[DM-XXX] 文档更新：功能描述`
