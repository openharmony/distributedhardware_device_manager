# ext/pin_auth 指引

> 本文件适用于 `ext/pin_auth/`。根目录 `AGENTS.md` 仍是知识库总入口，本文件只补充 PIN 认证扩展规则。

## 任务分类

- 本目录承载可插拔 PIN 认证模块，加载约定来自 `ext:ext_modules`。
- 改 PIN 生成、展示、校验、生命周期或弹框交互时，先确认主服务侧调用链和 SDK 入参语义。

## 必读路径

| 场景 | 先读 |
|---|---|
| 改 PIN 认证流程 | `docs/04-workflows/07-bind-unbind.md` |
| 改导入 PIN 免弹窗绑定 | `docs/04-workflows/08-import-pin-no-popup-bind.md` |
| 排查板端 PIN 弹框或推包验证 | `docs/06-debugging.md` |
| 改 JS API 相关展示链路 | `docs/05-interfaces/01-external-api.md` |

## 约束

- PIN 值禁止明文打印、落 hisysevent、写 PR 描述或测试日志。
- 不改变可插拔模块加载约定，除非同步评估 `ext:ext_modules` 和构建依赖。
- 不绕过服务侧权限和认证状态机直接建立可信关系。
- JSON 解析必须使用 `json/json_object.h`，先判断 `IsDiscarded`。
- 不使用 `std::stoi`、try-catch 或依赖异常的控制流。
- 不为调试写死 PIN、超时、重试次数或 feature flag。

## 验证

- C++ 改动至少编译 `device_manager` 或受影响的 pin_auth target。
- PIN 流程改动需要跑关联 UT；能上板时做真实 PIN 流程冒烟。
- 验证日志时必须确认没有明文 PIN 和敏感设备标识。
- 代码风格自检：单行不超过 120 字符，新增数值必须使用具名常量。
