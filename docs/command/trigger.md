# Trigger 命令使用教程

## 概述

Trigger 命令用于手动触发游戏中的各种事件，目前主要支持模拟NPC交互事件。该命令主要用于测试和调试场景。

## 基本命令格式

```
/trigger <事件类型> [参数...]
```

## 当前支持的事件

### 1. NPC交互事件 (npc.interact)

模拟玩家与NPC的交互行为，触发NPC的对话。

**语法**：
```
/trigger npc.interact <NPC名称> [对话条件]
```

**参数**：
- `<NPC名称>` - 要交互的NPC名称（必须已存在）
- `[对话条件]` - 可选，指定触发的对话条件（默认为"always"）

**示例**：
```
/trigger npc.interact shopkeeper
/trigger npc.interact guard on_duty
/trigger npc.interact wizard has_scroll
```

## 使用说明

### 基本用法

1. **触发默认对话**：
   ```
   /trigger npc.interact blacksmith
   ```
   这将显示blacksmith NPC设置的默认对话（对应"always"条件）

2. **触发条件对话**：
   ```
   /trigger npc.interact priest after_quest
   ```
   这将显示priest NPC在"after_quest"条件下设置的对话

3. **测试未定义条件的对话**：
   ```
   /trigger npc.interact villager unknown_condition
   ```
   如果条件未定义，NPC将显示默认的"..."对话

### 实际应用场景

1. **测试NPC对话系统**：
   ```
   # 设置对话
   /npc setdialogue bard default "欢迎来到小酒馆！"
   /npc setdialogue bard night "夜深了，要听首安眠曲吗？"
   
   # 测试对话
   /trigger npc.interact bard
   /trigger npc.interact bard night
   ```

2. **调试任务系统**：
   ```
   # 设置任务相关对话
   /npc setdialogue quest_giver before_quest "帮我找回丢失的项链"
   /npc setdialogue quest_giver after_quest "谢谢你！这是你的奖励"
   
   # 测试不同阶段对话
   /trigger npc.interact quest_giver before_quest
   /trigger npc.interact quest_giver after_quest
   ```

3. **检查对话完整性**：
   ```
   /trigger npc.interact all_villager1
   /trigger npc.interact all_villager2
   /trigger npc.interact all_villager3
   ```

## 注意事项

1. **NPC必须存在**：使用前确保NPC已通过`/npc create`命令创建
2. **对话条件区分大小写**：必须与设置时完全一致
3. **无副作用**：该命令仅触发对话显示，不会改变游戏状态
4. **主要用于测试**：建议在正式游戏中通过实际交互触发而非直接使用此命令

## 错误处理

| 错误情况 | 错误提示 | 解决方案 |
|----------|----------|----------|
| 参数不足 | "Usage: /trigger <event> [args...]" | 提供完整参数 |
| 未知事件类型 | "未知事件: xxx" | 使用支持的事件类型 |
| NPC不存在 | "NPC不存在: xxx" | 检查NPC名称或先创建NPC |