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

## 扩展建议

未来版本可能支持更多触发事件类型，如：
- 物品使用事件
- 区域进入事件
- 战斗触发事件
- 任务完成事件

## 常见问题

**Q：如何查看NPC有哪些对话条件？**
A：目前需要通过游戏引擎内部数据查看，未来可能添加查询命令。

**Q：触发对话会影响NPC状态吗？**
A：不会，这只是一个模拟交互的测试命令。

**Q：可以一次触发多个NPC的对话吗？**
A：当前版本不支持，需要多次执行命令。

**Q：为什么有些条件对话不显示？**
A：请确认已使用`/npc setdialogue`正确设置了该条件下的对话内容。

**Q：这个命令玩家可以使用吗？**
A：建议仅在开发测试时使用，正式版本应禁用或限制权限。
(纠正:玩家可以自由使用\[笑\])