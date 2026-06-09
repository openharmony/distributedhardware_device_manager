# 3rd 指引

> 本文件适用于 `3rd/`。根目录 `AGENTS.md` 仍是知识库总入口，本文件只补充第三方平行栈规则。

## 任务分类

- `3rd/` 面向非系统调用方，结构与主栈 `interfaces/`、`services/`、`utils/` 平行。
- 修改共享概念、错误码语义、SDK 入参语义、认证/绑定行为时，必须先查主栈对应实现。
- 只改 `3rd/` 而不同步主栈，或只改主栈而不同步 `3rd/`，都需要在最终回复说明原因。

## 必读路径

| 场景 | 先读 |
|---|---|
| 改 3rd SDK 接口 | 主栈 `interfaces/` 对应文件 + `docs/05-interfaces/01-external-api.md` |
| 改 3rd 服务实现 | 主栈 `services/` 对应文件 + `docs/05-interfaces/02-internal-api.md` |
| 改绑定/解绑共享语义 | `docs/04-workflows/07-bind-unbind.md` |
| 改导入 PIN / 代理绑定语义 | `docs/04-workflows/08-import-pin-no-popup-bind.md` |
| 改开源/闭源差异处理 | `docs/09-opensource-closedsource-isolation.md` |

## 约束

- 不要把 3rd 栈当成独立产品线；共享协议、字段、错误码和生命周期语义应与主栈一致。
- 不新增主栈没有评估过的第三方依赖。
- 不明文打印 `networkId`、`udid`、`uuid`、`deviceId`、PIN 或位置信息。
- JSON 解析必须使用仓内封装，先判断 `IsDiscarded`。
- 不使用 `std::stoi`、try-catch 或依赖异常的控制流。
- 改公开头或 SDK 语义时，按根目录 `Ask before` 规则升级评审。

## 验证

- C++ 改动至少编译受影响的 3rd target；无法定位 target 时编译 `device_manager_fwk` 或 `device_manager`。
- 共享概念改动要同时检查主栈 diff，最终回复列出是否同步及原因。
- 接口语义改动要说明兼容性影响和下游 include 路径影响。
- 代码风格自检：单行不超过 120 字符，新增数值必须使用具名常量。
