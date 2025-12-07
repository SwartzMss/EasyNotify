# EasyNotify

[![Build Status](https://github.com/SwartzMss/EasyNotify/actions/workflows/build.yml/badge.svg)](https://github.com/SwartzMss/EasyNotify/actions/workflows/build.yml)
[![Release Status](https://github.com/SwartzMss/EasyNotify/actions/workflows/release.yml/badge.svg)](https://github.com/SwartzMss/EasyNotify/actions/workflows/release.yml)

EasyNotify 是一款使用 Qt 编写的 Windows 桌面提醒工具，程序在系统托盘常驻，可在右下角以自定义弹窗方式显示提醒信息。

## 功能特性

- **托盘图标**：启动后最小化到托盘，可通过右键菜单显示主界面、开启勿扰模式或退出程序。
- **提醒管理**：在主界面中查看、搜索、添加或删除提醒。当前版本支持一次性、每日以及自动跳过节假日的工作日提醒。
- **自定义弹窗**：提醒到期时在右下角弹出小窗口，自动淡入淡出。
- **弹窗内容**：弹窗会展示提醒的优先级图标和正文。当前版本使用普通图标并留空正文，未来可在代码中传入自定义数据。
- **持久化配置**：所有提醒信息保存到内置的 SQLite 数据库 `config.db`，程序重启后会自动加载。

## 编译

项目基于 Qt 6。以 Windows 平台为例，使用 `qmake` 生成项目文件后通过 `nmake` 编译：

```bash
qmake
nmake
```

也可以参考仓库中的 [GitHub Actions 配置](.github/workflows/build.yml) 了解完整的构建流程。

## 运行

编译完成后运行生成的 `EasyNotify.exe`。第一次启动会在程序目录下创建 `config.db`，其中保存了提醒列表、暂停状态等信息。

## 配置存储与结构

配置数据存放在 SQLite 数据库 `config.db` 中（使用 Qt SQL API 读取/写入），无需再维护 JSON 配置，也不再兼容旧版 JSON 格式。核心字段：

- `isPaused`：是否暂停提醒
- `autoStart`：开机启动
- `soundEnabled`：声音提示
- `remoteUrl` / `remotePort`：远程触发地址
- `reminders` 表字段：`id`、`name`、`type`、`priority`、`nextTrigger`、`completed`

提醒类型：`0` 一次性；`1` 每日；`2` 工作日（跳过周末、法定节假日与调休补班）。优先级：`0` 低、`1` 中、`2` 高。

### 工作日数据

工作日提醒会优先读取程序目录下的 `workdays.json`，若不存在则使用内置的 `data/workdays.json`。文件结构已改为按年份分组：

```json
{
  "years": {
    "2025": {
      "holidays": ["2025-10-01", "2025-10-02"],
      "makeupDays": ["2025-09-28"]
    },
    "2026": {
      "holidays": ["2026-10-01"],
      "makeupDays": ["2026-10-10"]
    }
  },
  "version": 3
}
```

每个年份下的 `holidays` 视为法定节假日，`makeupDays` 视为调休补班日。可按年度维护，更新后重启程序生效。

## 远程触发

程序启动后会根据 `remoteUrl` 配置通过 TCP 主动连接到远程服务，
从该连接接收文本或 JSON 消息并在桌面弹出提醒。例如服务端发送一条
字符串即可在桌面看到通知。

## 许可证

本项目遵循 MIT License，详见 [LICENSE](LICENSE)。
