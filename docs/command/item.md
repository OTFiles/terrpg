# Item 命令使用教程

Item 命令是用于管理游戏物品系统的强大工具，包含定义物品、设置物品属性和给予物品等功能。

## 基本语法

```
/item <子命令> [参数...]
```

## 子命令列表

### 1. 定义物品 (`define`)

创建一个新物品类型。

**语法**:
```
/item define <物品名称> [参数...]
```

**参数**:
- `type` - 物品类型 (默认为 "generic")
- `display` - 物品显示字符 (默认为 '$')
- `pickupable` - 是否可拾取 (0/1, 默认为 0)
- `stackable` - 是否可堆叠 (0/1, 默认为 1)
- `value` - 物品价值 (默认为 10)
- 其他自定义属性

**示例**:
```
/item define sword type=weapon display=⚔️ damage=10 pickupable=1
/item define potion display=! stackable=1 value=50
```

### 2. 设置物品属性 (`setproperty`)

修改已定义物品的属性。

**语法**:
```
/item setproperty <物品名称> <属性=值>...
```

**特殊属性处理**:
- `damage` - 作为字符串存储
- `pickupable` - 可接受 "true"/"false"、"1"/"0"、"是"/"否"
- `stackable` 和 `value` - 直接设置

**示例**:
```
/item setproperty sword damage=15 pickupable=0
/item setproperty potion value=75 stackable=0
```

### 3. 给予物品 (`give`)

向玩家物品栏添加物品。

**语法**:
```
/item give <物品名称> [数量=1]
```

**说明**:
- 如果物品是 `stackable` 的，会尝试堆叠到现有物品上
- 否则会创建新的物品实例
- 数量可以通过位置参数或命名参数指定

**示例**:
```
/item give sword 1
/item give potion amount=5
```

## 高级用法

1. **自定义属性**:
   可以添加任意自定义属性到物品上，这些属性可以在游戏逻辑中使用。

   ```
   /item define key display=🔑 special_effect=unlock_door
   ```

2. **物品类型系统**:
   通过 `type` 参数可以创建不同类型的物品，游戏系统可以根据类型应用不同的行为。

   ```
   /item define armor type=equipment display=🛡️ defense=5
   ```

3. **批量修改**:
   可以一次性修改多个属性。

   ```
   /item setproperty sword damage=12 value=200 pickupable=1
   ```

## 注意事项

1. 物品名称是唯一的，定义重复名称会覆盖原有物品
2. 修改物品属性会影响所有该类型的物品实例
3. 给予物品时会自动处理堆叠逻辑
4. 使用前请确保物品已定义

## TODO
1. 添加物品删除功能 (`/item remove <name>`)
2. 查看物品详细信息(允许对物品的详细信息进行定义)
3. 设置显示颜色