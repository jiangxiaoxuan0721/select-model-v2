# OpenHands CLI 模块结构图表

## 模块组件关系图

```mermaid
graph TD
    A[main.py] --> B[commands.py]
    A --> C[tui.py]
    B --> C
    B --> D[utils.py]
    B --> E[settings.py]
    B --> F[shell_config.py]
    B --> G[vscode_extension.py]
    B --> H[suppress_warnings.py]
    C --> D

    subgraph "核心功能"
        A
    end

    subgraph "命令处理"
        B
    end

    subgraph "用户界面"
        C
    end

    subgraph "辅助功能"
        D
        E
        F
        G
        H
    end
```

## 命令处理流程

```mermaid
sequenceDiagram
    participant User
    participant Main as main.py
    participant Commands as commands.py
    participant TUI as tui.py
    participant EventStream

    User->>Main: 输入命令
    Main->>Commands: handle_commands()

    alt /help 命令
        Commands->>TUI: display_help()
        TUI-->>User: 显示帮助信息
    else /exit 命令
        Commands->>TUI: 确认退出
        TUI-->>User: 显示确认对话框
        User->>TUI: 确认
        Commands->>EventStream: 添加停止事件
        Commands-->>Main: 返回关闭REPL标志
    else /status 命令
        Commands->>TUI: display_status()
        TUI-->>User: 显示状态信息
    else /new 命令
        Commands->>TUI: 确认新会话
        TUI-->>User: 显示确认对话框
        User->>TUI: 确认
        Commands->>EventStream: 添加停止事件
        Commands-->>Main: 返回关闭REPL和新会话标志
    else /settings 命令
        Commands->>TUI: display_settings()
        TUI-->>User: 显示设置选项
        User->>TUI: 选择设置类型
        Commands->>TUI: 修改设置
    else /mcp 命令
        Commands->>TUI: 显示MCP选项
        TUI-->>User: 显示MCP菜单
        User->>TUI: 选择操作
        Commands->>TUI: 执行MCP操作
    else 普通消息
        Commands->>EventStream: 添加消息事件
        Commands-->>Main: 返回关闭REPL标志
    end

    Main-->>User: 处理结果
```

## 文件功能概述

```mermaid
classDiagram
    class main {
        +main()
        +run_cli()
        +setup_repl()
        +handle_user_input()
    }

    class commands {
        +handle_commands()
        +handle_exit_command()
        +handle_help_command()
        +handle_status_command()
        +handle_new_command()
        +handle_settings_command()
        +handle_mcp_command()
        +handle_resume_command()
        +handle_init_command()
    }

    class tui {
        +display_help()
        +display_status()
        +display_shutdown_message()
        +cli_confirm()
        +read_prompt_input()
        +create_prompt_session()
        +display_mcp_errors()
    }

    class utils {
        +read_file()
        +write_to_file()
        +get_local_config_trusted_dirs()
        +add_local_config_trusted_dir()
    }

    class settings {
        +display_settings()
        +modify_llm_settings_basic()
        +modify_llm_settings_advanced()
    }

    class shell_config {
        +setup_shell_alias()
        +remove_shell_alias()
    }

    class vscode_extension {
        +check_vscode_extension()
        +install_vscode_extension()
    }

    class suppress_warnings {
        +suppress_common_warnings()
    }

    main --> commands
    main --> tui
    commands --> tui
    commands --> utils
    commands --> settings
    commands --> shell_config
    commands --> vscode_extension
    commands --> suppress_warnings
```

## MCP服务器管理流程

```mermaid
flowchart TD
    A[开始] --> B{选择操作}
    B -->|列出服务器| C[display_mcp_servers]
    B -->|添加服务器| D{选择服务器类型}
    B -->|删除服务器| E[remove_mcp_server]
    B -->|查看错误| F[display_mcp_errors]

    D -->|SSE| G[add_sse_server]
    D -->|Stdio| H[add_stdio_server]
    D -->|SHTTP| I[add_shttp_server]

    G --> J[收集服务器信息]
    H --> J
    I --> J

    J --> K[验证配置]
    K -->|验证失败| J
    K -->|验证成功| L[保存到配置文件]

    L --> M{是否重启CLI}
    M -->|是| N[restart_cli]
    M -->|否| O[结束]

    C --> O
    E --> M
    F --> O
```

## CLI命令概述

| 命令 | 描述 | 处理函数 |
|------|------|----------|
| `/help` | 显示帮助信息 | `handle_help_command()` |
| `/exit` | 退出当前会话 | `handle_exit_command()` |
| `/status` | 显示当前状态 | `handle_status_command()` |
| `/new` | 开始新会话 | `handle_new_command()` |
| `/settings` | 修改设置 | `handle_settings_command()` |
| `/mcp` | 管理MCP服务器 | `handle_mcp_command()` |
| `/resume` | 恢复当前任务 | `handle_resume_command()` |
| `/init` | 初始化仓库 | `handle_init_command()` |
