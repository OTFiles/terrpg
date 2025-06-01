# Entity

## 概述

Entity 命令用于管理游戏中的各种实体（NPC、物品、地图对象等）的属性设置。

## 基本命令格式

```
/entity set <实体名称> <属性名> <属性值>
```

## 功能说明

### 设置实体属性

修改指定实体的属性值。

**语法**：
```
/entity set <实体名称> <属性名> <属性值>
```

**参数**：
- `<实体名称>` - 要修改的实体名称（NPC、物品或地图对象的名称）
- `<属性名>` - 要设置的属性名称
- `<属性值>` - 要设置的属性值（可以包含空格）

**支持的实体类型**：
1. NPC角色
2. 物品
3. 地图上的对象

## 使用示例

### 1. 修改NPC属性

```
/entity set guard health 100
/entity set merchant dialog "欢迎光临我的商店！"
/entity set king display K
```

### 2. 修改物品属性

```
/entity set sword damage 15
/entity set potion effect "恢复50点生命值"
/entity set key display %
```

### 3. 修改地图对象属性

```
/entity set treasure_chest locked true
/entity set secret_door visible false
/entity set magic_fountain effect "恢复所有法力"
```

## 注意事项

1. 实体名称区分大小写，必须与创建时完全一致
2. 如果属性不存在，将会被创建
3. 修改地图对象属性前，请确保对象已存在于某张地图上