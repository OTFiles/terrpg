# NPC

## 概述

NPC 命令用于创建和管理游戏中的非玩家角色(NPC)，包括创建NPC和设置对话内容。

## 基本命令格式

```
/npc <子命令> [参数...]
```

## 子命令详解

### 1. 创建NPC

创建一个新的NPC角色。

**语法**：
```
/npc create <NPC名称> [template=模板名称]
```

**参数**：
- `<NPC名称>` - NPC的唯一标识名称（必填）
- `template` - 可选，指定使用的NPC模板（默认为基础NPC）

**示例**：
```
/npc create villager
/npc create guard template=soldier
/npc create merchant template=shopkeeper
```

### 2. 设置NPC对话

为NPC设置特定条件下的对话内容。

**语法**：
```
/npc setdialogue <NPC名称> <触发条件> <对话内容>
```

**参数**：
- `<NPC名称>` - 要设置对话的NPC名称
- `<触发条件>` - 触发对话的条件（如"default", "quest1", "after_quest"等）
- `<对话内容>` - NPC要说的内容（可以包含空格）

**示例**：
```
/npc setdialogue villager default "你好，旅行者！"
/npc setdialogue guard quest1 "城堡现在禁止进入"
/npc setdialogue merchant after_quest "谢谢你完成了任务！这是我的谢礼"
```
(注:教程是让ds写的，所以这个"旅行者"是不是说...)

## 使用示例

### 创建基础NPC
```
/npc create blacksmith
/npc setdialogue blacksmith default "需要打造武器吗？"
/npc setdialogue blacksmith has_iron "啊，你带来了铁矿石！我可以为你打造一把剑。"
```

### 使用模板创建NPC
```
/npc create royal_guard template=elite_guard
/npc setdialogue royal_guard default "止步！皇家禁地不得擅入"
/npc setdialogue royal_guard has_medal "向英雄致敬！请进。"
```

### 设置多行对话
```
/npc setdialogue bard default "让我为你唱首歌吧~ 玫瑰红似火，紫罗兰蓝如天..."
```

## 高级用法

1. **使用模板批量创建相似NPC**：
   ```
   /npc create guard1 template=city_guard
   /npc create guard2 template=city_guard
   /npc create guard3 template=city_guard
   ```

2. **设置多种触发条件的对话**：
   ```
   /npc setdialogue priest default "愿光明与你同在"
   /npc setdialogue priest night "夜晚危险，小心行事"
   /npc setdialogue priest injured "让我为你治疗伤口"
   ```

3. **结合游戏事件设置对话**：
   ```
   /npc setdialogue mayor before_attack "我们村庄一直很和平"
   /npc setdialogue mayor after_attack "那些怪物毁了一切..."
   ```

## 注意事项

1. NPC名称必须唯一，重复创建会覆盖现有NPC
2. 对话内容可以包含空格，不需要引号
3. 触发条件区分大小写
4. 要删除对话，可以设置为空字符串：
   ```
   /npc setdialogue npc_name condition ""
   ```
5. 模板NPC需要预先在游戏引擎中定义

## 常见问题

**Q：如何查看已创建的NPC？**
A：目前需要通过游戏引擎内部方法查看，未来可能添加列表命令。

**Q：对话中可以包含特殊符号吗？**
A：可以，但某些符号可能需要转义处理。

**Q：如何让NPC根据玩家状态改变对话？**
A：通过设置不同的触发条件，游戏引擎会在适当时机自动选择匹配的对话。

**Q：NPC可以移动吗？**
A：移动功能需要通过其他命令或脚本实现，本命令仅处理创建和对话设置。