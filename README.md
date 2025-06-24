# EasyNotify

[![Build Status](https://github.com/SwartzMss/EasyNotify/actions/workflows/build.yml/badge.svg)](https://github.com/SwartzMss/EasyNotify/actions/workflows/build.yml)
[![Release Status](https://github.com/SwartzMss/EasyNotify/actions/workflows/release.yml/badge.svg)](https://github.com/SwartzMss/EasyNotify/actions/workflows/release.yml)

EasyNotify 是一款使用 Qt 编写的 Windows 桌面提醒工具，程序在系统托盘常驻，可在右下角以自定义弹窗方式显示提醒信息。

## 功能特性

- **托盘图标**：启动后最小化到托盘，可通过右键菜单显示主界面、开启勿扰模式或退出程序。
- **提醒管理**：在主界面中查看、搜索、添加或删除提醒。当前版本支持一次性提醒和每日提醒。
- **自定义弹窗**：提醒到期时在右下角弹出小窗口，自动淡入淡出。
- **弹窗内容**：弹窗会展示提醒的优先级图标和正文。当前版本使用普通图标并留空正文，未来可在代码中传入自定义数据。
- **持久化配置**：所有提醒信息保存到 `config.json`，程序重启后会自动加载。
- **导入导出**：支持将提醒列表保存为 JSON 文件或从 JSON 文件导入。

## 编译

项目基于 Qt 6。以 Windows 平台为例，使用 `qmake` 生成项目文件后通过 `nmake` 编译：

```bash
qmake
nmake
```

也可以参考仓库中的 [GitHub Actions 配置](.github/workflows/build.yml) 了解完整的构建流程。

## 运行

编译完成后运行生成的 `EasyNotify.exe`。第一次启动会在程序目录下创建 `config.json`，其中保存了提醒列表和暂停状态等信息。

## 配置文件格式

`config.json` 大致结构如下：

```json
{
  "isPaused": false,
  "remoteUrl": "tcp://example.com:12345",
 "reminders": [
    {
      "id": "uuid",
      "name": "示例提醒",
      "type": 0,
      "priority": 1,
      "nextTrigger": "2025-01-01T09:00:00",
      "completed": false
    }
 ]
}
```

其中 `type` 字段为 `0` 表示一次性提醒，`1` 表示每日提醒。`priority` 字段的取值为 `0`（低）、`1`（中）、`2`（高）。

## 远程触发

程序启动后会根据 `remoteUrl` 配置通过 TCP 主动连接到远程服务，
从该连接接收文本或 JSON 消息并在桌面弹出提醒。例如服务端发送一条
字符串即可在桌面看到通知。

## 许可证

本项目遵循 MIT License，详见 [LICENSE](LICENSE)。
