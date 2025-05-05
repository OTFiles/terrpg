# TerRPG

## 概述

这是一个基于C++和ncurses库开发的文字冒险游戏引擎，支持一些基础功能。

## 功能特性

### 核心系统
- **地图系统**：支持多地图创建、传送和视口渲染
- **物品系统**：支持物品收集、使用、丢弃和堆叠
- **NPC系统**：支持NPC创建、对话交互和条件对话
- **变量系统**：支持游戏内变量存储和数学运算
- **条件系统**：支持复杂的条件判断逻辑

### 命令系统
- `/map` - 地图管理命令
- `/npc` - NPC管理命令
- `/entity` - 实体属性管理命令
- `/scoreboard` - 变量管理命令
- `/teleport` - 传送命令
- `/trigger` - 事件触发命令

## 快速开始

### 编译运行
1. 确保安装ncurses库
2. 编译项目：
   ```bash
   mkdir build && cd build
   cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .
   # 启用多线程以加速编译
   cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . -j4
   ```
3. 运行游戏：
   ```bash
   ./bin/GameEngine
   ```
   确保运行时当前目录下存在game.txt文件

### 游戏控制
- **方向键**：移动角色
- **g键**：拾取物品
- **u键**：与NPC对话
- **i键**：打开物品栏
- **ESC键**：返回上一级菜单
- **q键**：退出游戏

## 游戏数据格式

游戏数据通过`game.txt`文件加载，支持以下结构：

```plaintext
init {
    // 初始化命令
    /map create main 30 20
    /npc create guard
    /scoreboard add kills
}

item 使用效果:sword {
    /scoreboard operation kills += 1
    /showDialog "系统" "你击败了敌人"
}

if have sword {
    /npc setdialogue blacksmith has_sword "我看到你有一把好剑"
}
```

## 开发者指南

### 核心类说明

1. **GameEngine** - 游戏引擎主类
   - 管理游戏状态、变量、地图和实体
   - 处理输入和渲染
   - 执行游戏逻辑

2. **GameMap** - 地图类
   - 存储地图数据和对象
   - 提供寻路和碰撞检测

3. **GameObject** - 游戏对象基类
   - 表示NPC、物品、陷阱等游戏实体
   - 支持自定义属性和行为

### 扩展游戏功能

1. **添加新命令**：
   - 继承`Command`基类
   - 实现`handle`方法
   - 在`CommandParser`中注册

2. **添加新游戏机制**：
   - 修改`GameEngine`类
   - 扩展`GameObject`属性系统
   - 添加新的条件判断类型

## 示例游戏脚本

```plaintext
// 初始化游戏世界
init {
    /map create village 40 30
    /map create dungeon 50 50
    
    // 创建NPC
    /npc create blacksmith
    /npc setdialogue blacksmith default "需要打造武器吗？"
    
    // 创建物品
    /entity set sword damage 10
    /entity set sword display !
    
    // 设置初始变量
    /scoreboard add gold
    /scoreboard set gold 100
}

// 剑的使用效果
item 使用效果:sword {
    /showDialog "系统" "你挥舞了剑！"
}

// 条件内容
if have sword {
    /npc setdialogue blacksmith has_sword "我看到你已经有一把剑了"
}
```

## 注意事项

1. 地图坐标从(0,0)开始
2. 所有名称区分大小写
3. 变量系统仅支持整数运算
4. 游戏数据文件使用UTF-8编码
5. 需要支持宽字符的终端环境

## 常见问题

Q: 为什么地图显示不正常？
A: 请确保终端支持UTF-8和宽字符显示，并安装了合适的字体。

Q: 如何调试游戏脚本？
A: 编译时定义DEBUG宏可以看到详细的加载日志。

Q: 物品使用效果不触发？
A: 检查物品名称是否匹配，以及使用效果是否正确定义。