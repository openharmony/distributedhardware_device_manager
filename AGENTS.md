# 分布式设备管理（device_manager）指引

## 项目定位

DeviceManager 是 OpenHarmony `foundation/distributedhardware/distributedhardware_device_manager` 下的分布式设备管理部件，提供账号无关的分布式设备认证组网能力，对外暴露设备监听、发现、认证、绑定/解绑接口，对内依赖 dsoftbus 提供设备上下线/发现/通道、deviceauth 提供群组认证。本仓不是 dsoftbus 或 deviceauth 的替代实现，也不负责 DP（distributed_device_profile）数据 schema 设计。

优先按这些目录定位问题：

- `services/implementation/src/` — 服务核心实现：认证状态机、ACL 写入、发现/发布、依赖（softbus/hichain/commonevent/multipleuser）。**（高频改动）**
- `services/implementation/src/authentication_v2/` — 新协议认证实现。**（高频改动，与老协议隔离）**
- `services/service/` — DM SA 入口与 IPC 桩/代理实现。
- `interfaces/inner_kits/native_cpp/` — 对外 SDK 与 NAPI/NDK/CJ 公开出口，IPC 回调通知。**（高风险，ABI 边界）**
- `ext/pin_auth/` 与 `3rd/` — PIN 认证扩展与第三方栈共享语义。

### 常见任务速查

| 任务类型 | 首先查看 |
|---|---|
| 认证协议变更（新增/改流程） | `services/implementation/src/authentication_v2/`（新协议）；`services/implementation/src/authentication/`（老协议，勿交叉） |
| IPC 桩/代理方法增改 | `services/service/`；同步改 `interfaces/inner_kits/native_cpp/` 回调 |
| SDK/NAPI/NDK/CJ 公开出口变更 | `interfaces/inner_kits/native_cpp/`；确认 `bundle.json` 的 `inner_kits` 已登记 |
| ACL 写入/清理/生命周期 | `services/implementation/`；读 `services/implementation/AGENTS.md` |
| PIN 认证细节 | `ext/pin_auth/`；读 `ext/pin_auth/AGENTS.md` |
| 发现/发布问题 | `services/implementation/src/` 中 discover/publish 相关实现 |
| 权限白名单/黑名单/系统应用判断 | `permission/dm_permission.json` + `services/implementation/` 中权限校验逻辑 |

## 构建和验证

构建命令在 OpenHarmony 源码根目录执行，不在本仓子目录执行。本仓没有 npm、cargo、Makefile，不要发明 make / cmake 命令。常用目标、minimum checks、完成定义详见 `docs/03-build-test.md`。

```sh
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager_fwk
./build.sh --product-name <product> --ccache --build-target foundation/distributedhardware/device_manager:device_manager_test
```

静态检查与 API 兼容性（详见 `docs/03-build-test.md` §4-§5）：

- 编译零警告：部分 target 的 `cflags` 含 `-Wall -Werror`，所有 C++ 改动必须零警告通过编译。
- 代码格式：改文件后执行 `clang-format --dry-run --Werror <file>` 确认无格式差异。
- 涉及公开 SDK 头签名、错误码或 IPC code 变更时，需确认无 ABI break：检查 `interfaces/inner_kits/native_cpp/` 头文件签名未删改已有接口，IPC code 数值未变（见"IPC 和接口码约束"）。

特殊说明：分布式验证两板必须同步推包、同步重启、等待 BOOT_COMPLETED 并确认两端亮屏、同 Wi-Fi；改 `services/service`、`services/implementation`、`common`、`interfaces/inner_kits` 后板端验证前必须推对应 `.so` 并重启，不能只跑新测试二进制。PR 必须填写安全自检、TDD 结果、XTS 结果；涉及 IPC、SDK 头、`bundle.json`、SA、权限、hisysevent 的改动需在文档中说明。

## 知识索引

| 场景 | 先读 |
|---|---|
| 发布发现、绑定解绑、Import PIN 免弹框绑定、权限白名单 / 黑名单 / 系统应用判断 | `docs/01-workflows.md` |
| DP / ACL 持久化验证、ACL 环境清理、推包验证、WSL / 双板、云端镜像 / 凭据 | `docs/02-verification.md` |
| 构建入口、minimum checks、完成定义、无法验证时的说明 | `docs/03-build-test.md` |
| PIN 认证细节 | `ext/pin_auth/AGENTS.md` |
| 3rd 栈共享语义 | `3rd/AGENTS.md` |
| ACL 写入 / 清理 / 永久可信关系生命周期 | `services/implementation/AGENTS.md` |

### 术语路由

任务描述、日志、issue、API 或文件中出现以下术语时，先读对应文档再动手：

| 术语 / 缩写 | 含义 | 先读 |
|---|---|---|
| ACL / `AccessControlProfile` | 访问控制表，定义设备间可信关系 | 本文件"架构约束"；`services/implementation/AGENTS.md` |
| DP / distributed_device_profile | 分布式设备档案服务，DM 依赖其持久化 ACL | `docs/02-verification.md`（DP RDB 验证方法） |
| sink / source | 绑定两端角色：source 主动发起、sink 被动接受 | 本文件"时序约束"；`docs/01-workflows.md` §2 |
| Import PIN / ExportAuthInfo | 免弹框 PIN 绑定流程 | `docs/01-workflows.md` §3 |
| authentication_v2 / authentication | 新/老认证协议实现路径 | 本文件"架构约束"协议隔离规则 |
| dsoftbus / softbus | 设备发现与传输通道，DM 依赖其上下线/发现/通道 | 本文件"项目定位" |
| deviceauth / hichain | 群组认证组件，提供群组认证能力 | 本文件"项目定位" |
| `ACL_LIFE_CYCLE_DAYS` | 单条可信关系的生命周期属性 | 本文件"架构约束"；`docs/01-workflows.md` §3 |
| `bundle.json` `inner_kits` | 公开 SDK 出口登记表 | 本文件"IPC 和接口码约束" |
| hisysevent | 系统事件埋点 | 本文件"需先升级确认的改动" |

编辑前必须声明：任务类别、已读文档、已发现约束。

## 开发约束

### 需先升级确认的改动

修改以下内容前先让开发者人工确认：

- IPC code
- `bundle.json` 的 `inner_kits` / `deps.components`
- `sa_profile/` 或 init 脚本
- `permission/dm_permission.json`
- hisysevent 事件
- 顶层 `BUILD.gn` group 或 `device_manager.gni` feature flag
- 兼容 fallback / migration / deprecated 路径
- 第三方组件依赖
- 第三方组件许可证类型变更（如引入 GPL 类组件或 MIT → Apache 切换），需法务确认
- 公开 SDK 头签名、错误码或生命周期语义
- 老协议的对外行为，协议路径：`services/implementation/src/authentication`
- 新协议的对外行为，协议路径：`services/implementation/src/authentication_v2`

### 架构约束

- **记录原则**：本文只记录 AI 不能从源码稳定推断、但执行任务时必须遵守的规则。接口签名、目录职责、调用链、枚举值和路径清单以源码为准。违反后果：规则与源码漂移，AI 依据过期规则改码导致行为不一致。
- **sink/source 职责边界**：sink/source 职责必须清晰；跨角色改动必须说明双方影响。违反后果：双端行为不一致，绑定/解绑状态错乱。
- **协议隔离**：新增认证能力默认走 `authentication_v2`，不要顺手改老协议 `authentication`；`authentication_v2` 与老协议路径 `services/implementation/src/authentication/` 互不交叉，不要把新协议 helper 抄进老协议。违反后果：协议行为漂移，兼容性与回滚能力受损。
- **兼容路径**：兼容 fallback、migration、deprecated 路径改动前需升级确认。违反后果：老设备/老协议回归路径被破坏，无法回滚。
- **ACL 关系属性**：`ACL_LIFE_CYCLE_DAYS` 是单条可信关系的生命周期，不是设备属性；关系级属性只写 `AccessControlProfile.extraData`，不得写 `accesser/accessee.extraData`。违反后果：关系级与账户级语义混淆，ACL 查询/同步错配。
- **ACL 持久化**：ACL / `extraInfo` 只持久化输入参数，不冗余写派生快照；DP schema 不变，新增字段优先走 JSON `extraData`。违反后果：派生快照与输入参数漂移，端到端状态不一致。
- **生成代码边界**：本仓当前无自动生成代码；若后续引入代码生成器，生成产物的 source-of-truth 必须在 `BUILD.gn` 中声明，禁止手改生成产物。违反后果：生成产物被手改后，下次重新生成覆盖手动改动，产生难以排查的回归。

### Clean Code 规范

- **设备标识与敏感信息**：`networkId`、`udid`、`uuid`、`deviceId`、PIN、位置信息都按敏感信息处理，不得明文写入日志、事件、PR、issue。违反后果：隐私泄露与合规风险。
- **JSON 输入解析**：`LocalServiceInfo`、`ImportAuthInfo`、`extraInfo` 等 JSON 输入解析后先判 `IsDiscarded`；`JsonObject` 复制使用 `Duplicate()`；不要绕过 `json/json_object.h` 直接 include nlohmann/cJSON。违反后果：解析逻辑与废弃标记脱节，JSON 内存语义不一致。
- **无异常构建**：全仓构建带 `-fno-exceptions`：禁止 try-catch、`std::stoi`。由于 `std::sto*` 在输入非法（非数字或溢出）时会抛出 `std::invalid_argument` 或 `std::out_of_range` 异常，而项目禁止异常处理（无 try-catch），异常一旦触发，C++ 运行时将调用 `std::terminate()` → `abort()` 直接终止进程，远端设备可借此发起拒绝服务；`std::stoi`、`std::stoll`、`std::stou` 等会抛异常的转换函数须改用 `atoi`、`atoll`、`atou` 等无异常抛出的替代方案。违反后果：远端可通过构造非法输入触发进程崩溃，形成 DoS。
- **权限校验元信息**：`ImportAuthInfo` / `ExportAuthInfo` 校验的是 IPC 元信息中的实际 process name，不是 pkgName；白名单、黑名单、系统应用判断不要用硬编码绕过。违反后果：以 pkgName 做判断会让白名单失效，未授权进程可调用受控接口。

### IPC 和接口码约束

- **IPC code 是 ABI**：IPC code 数值是 ABI，不得修改已有值。违反后果：破坏二进制兼容，旧客户端调用错位。
- **公开出口登记**：SDK 头、NAPI、NDK、CJ 等公开出口必须经 `bundle.json` 的 `inner_kits` 显式登记。违反后果：未登记接口被外部依赖后无法追踪，后续重构会破坏调用方。
- **公开语义变更**：改公开 SDK 头签名、错误码或生命周期语义前需升级确认。违反后果：调用方未同步升级即出现编译/运行期不兼容。

### 时序约束

- **发布/发现时序**：DM 发布/发现依赖屏幕事件与 Wi-Fi/蓝牙状态，必须保持双端设备屏幕亮屏并且 Wi-Fi/蓝牙开启。违反后果：任一端灭屏或 Wi-Fi/蓝牙关闭都可能导致发现/发布功能失败。
- **绑定 source/sink 时序**：绑定涉及 source（主动发起端）与 sink（被动接受端）；当一端状态、参数、ACL 写入修改时，必须同步确认修改对端行为，保持双端行为一致。违反后果：双端行为不一致，绑定状态错乱。
- **Import PIN 免弹框时序**：必须先 `ExportAuthInfo` 再 `ImportAuthInfo`；PIN 单次有效，bind 完成或 PIN timer 到期后重跑必须重新 Export + Import；新接口路径下 `ImportAuthInfo` 会写 LocalServiceInfo 并注册 PIN，不需要再额外调用 `RegisterLocalServiceInfo`。违反后果：PIN 失效导致绑定失败或卡死在等待输入状态。
- **Import PIN 一致性时序**：`Init(pkgName)`、`pinConsumerPkgName`、`bizSrcPkgName`、`bizSinkPkgName` 与 self token process name 必须保持一致；sink/source 的 `pinConsumerPkgName` 与 `pinExchangeType` 必须完全一致；`extraInfo` 携带 `ACL_LIFE_CYCLE_DAYS` 时 `authType` 必须是 `TRUST_ALWAYS`。违反后果：白名单与兼容分支不一致，入口校验拒绝或字段不落盘。
- **永久可信关系清理时序**：必须逐条 ACL 读取生命周期判断，不得用全局默认值覆盖指定生命周期。违反后果：指定生命周期的可信关系被误清理或永久残留。
- **分布式验证时序**：两板必须同步推包、同步重启、等待 BOOT_COMPLETED。违反后果：旧 `.so` 与新代码混跑，验证结论无效。
