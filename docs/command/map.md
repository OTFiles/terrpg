# Map

## 概述

Map 命令用于创建和编辑游戏地图，包括创建新地图、设置单个方块和填充区域等功能。

## 命令格式

基本格式：`/map <子命令> [参数...]`

## 子命令

### 1. 创建地图

创建一个新的地图。

**语法**：
```
/map create <地图名称> [width=20] [height=20]
```

**参数**：
- `<地图名称>` - 地图的唯一标识名称
- `width` - 可选，地图宽度，默认20
- `height` - 可选，地图高度，默认20

**示例**：
```
/map create dungeon
/map create forest width=30 height=30
/map create cave width=50 height=10
```

### 2. 设置方块

在地图上设置特定位置的方块类型。

**语法**：
```
/map setblock <地图名称> <x> <y> <类型> [name=值] [display=字符] [其他属性...]
```

**参数**：
- `<地图名称>` - 要修改的地图名称
- `<x> <y>` - 方块的坐标位置
- `<类型>` - 方块类型（wall, npc, item, trap, marker等）
- `name` - 可选，方块名称（对于item类型必须指定）
- `display` - 可选，显示字符，默认根据类型自动选择
- 其他类型特定属性（如trap的damage）

**支持的方块类型**：
- `wall` - 墙壁（不可通行）
- `npc` - NPC角色
- `item` - 物品
- `trap` - 陷阱
- `marker` - 标记点

**示例**：
```
/map setblock dungeon 5 5 wall
/map setblock dungeon 10 10 npc name=guard display=G
/map setblock dungeon 15 15 item name=sword display=!
/map setblock dungeon 20 20 trap damage=15 display=^
```

### 3. 填充区域

用指定方块类型填充一个矩形区域。

**语法**：
```
/map fill <地图名称> <x1> <y1> <x2> <y2> <类型> [name=值] [display=字符] [其他属性...]
```

**参数**：
- `<地图名称>` - 要修改的地图名称
- `<x1> <y1>` - 区域起点坐标
- `<x2> <y2>` - 区域终点坐标
- `<类型>` - 方块类型（wall, trap等）
- `name` - 可选，方块名称
- `display` - 可选，显示字符
- 其他类型特定属性

**示例**：
```
/map fill dungeon 0 0 19 19 wall  # 用墙壁填充整个地图边界
/map fill dungeon 5 5 15 15 trap damage=5 display=^  # 填充陷阱区域
/map fill dungeon 10 10 12 12 item name=potion display=!  # 填充药水区域
```

## 使用技巧

1. **创建基础地图**：
   ```
   /map create mymap width=30 height=20
   /map fill mymap 0 0 29 19 wall
   /map fill mymap 1 1 28 18 floor
   ```

2. **添加房间**：
   ```
   # 创建房间1
   /map fill mymap 5 5 10 10 floor
   /map setblock mymap 5 5 wall
   /map setblock mymap 10 10 wall
   
   # 创建房间2并连接
   /map fill mymap 15 5 20 10 floor
   /map fill mymap 10 7 15 7 floor  # 走廊
   ```

3. **添加物品和NPC**：
   ```
   /map setblock mymap 7 7 item name=sword
   /map setblock mymap 17 8 npc name=merchant display=M
   ```

4. **添加陷阱**：
   ```
   /map fill mymap 12 5 12 10 trap damage=5
   ```

## 注意事项

1. 使用`setblock`设置物品时，必须确保物品已在引擎中定义
2. 坐标从0开始，必须在指定地图的范围内
3. 填充区域时，起点和终点坐标会自动调整为矩形区域
4. 默认显示字符：
   - 墙壁：`#`
   - NPC：`@`
   - 物品：`$`
   - 陷阱：`^`
   - 标记点：`*`