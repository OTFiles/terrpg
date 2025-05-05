# Scoreboard 命令使用教程

## 概述

Scoreboard 命令用于管理游戏中的计分板变量系统，可以创建变量、设置变量值以及对变量进行数学运算。

## 基本命令格式

```
/scoreboard <子命令> [参数...]
```

## 子命令详解

### 1. 添加变量 (add)

创建一个新的计分板变量并初始化为0。

**语法**：
```
/scoreboard add <变量名>
```

**参数**：
- `<变量名>` - 要创建的变量名称（区分大小写）

**示例**：
```
/scoreboard add kills
/scoreboard add player_score
/scoreboard add quest_progress
```

### 2. 设置变量值 (set)

设置变量的值（支持数学表达式）。

**语法**：
```
/scoreboard set <变量名> <值或表达式>
```

**参数**：
- `<变量名>` - 要设置的变量名称
- `<值或表达式>` - 可以是具体数值或数学表达式

**支持的运算符**：
- 基本运算：`+`, `-`, `*`, `/`
- 括号：`(`, `)`
- 可以使用其他变量值（用`$`前缀）

**示例**：
```
/scoreboard set kills 0
/scoreboard set player_score 100+50
/scoreboard set total $kills * 10
/scoreboard set bonus (100 + $player_score) / 2
```

### 3. 变量运算 (operation)

对变量进行数学运算操作。

**语法**：
```
/scoreboard operation <变量名> <运算符> <值或表达式>
```

**支持的运算符**：
- `=` - 赋值
- `+=` - 加法赋值
- `-=` - 减法赋值
- `*=` - 乘法赋值
- `/=` - 除法赋值

**示例**：
```
/scoreboard operation kills += 1          # 击杀数+1
/scoreboard operation score += $kills * 5 # 分数增加击杀数×5
/scoreboard operation health -= 10        # 生命值减少10
/scoreboard operation damage *= 1.5       # 伤害提升50%
```

## 使用示例

### 基础变量管理
```
# 创建变量
/scoreboard add kills
/scoreboard add score

# 设置初始值
/scoreboard set kills 0
/scoreboard set score 100

# 更新值
/scoreboard operation kills += 1
/scoreboard operation score += 50
```

### 复杂表达式
```
# 创建关联变量
/scoreboard add base_score
/scoreboard add multiplier
/scoreboard add final_score

# 设置值
/scoreboard set base_score 500
/scoreboard set multiplier 1.2

# 计算最终得分
/scoreboard set final_score $base_score * $multiplier
```

### 游戏内应用
```
# 玩家击杀敌人时
/scoreboard operation player_kills += 1
/scoreboard operation player_exp += 10

# 玩家升级时
/scoreboard operation player_level += 1
/scoreboard operation player_max_health *= 1.1
```

## 高级特性

1. **表达式嵌套**：
   ```
   /scoreboard set result (100 + $base) * ($multiplier - 0.5)
   ```

2. **多变量联动**：
   ```
   /scoreboard operation total_damage += $base_damage * $crit_multiplier
   ```

3. **条件运算**（需配合其他命令）：
   ```
   # 假设有if命令
   /if $score > 100 then /scoreboard set bonus 50
   ```

## 注意事项

1. 变量名称区分大小写
2. 引用其他变量时需加`$`前缀
3. 除数为零会导致错误
4. 未定义的变量不能直接使用（需先创建）
5. 表达式中的空格会被忽略（除非在引号内）

## 常见问题

**Q：如何查看所有变量？**
A：目前需要通过游戏引擎内部方法查看，未来可能添加list子命令。

**Q：变量值有范围限制吗？**
A：变量使用整数存储，范围为-2147483648到2147483647。

**Q：可以删除变量吗？**
A：当前版本不支持直接删除，可以通过设置为0来"重置"变量。

**Q：表达式支持哪些数学函数？**
A：当前版本支持基本四则运算，未来可能扩展更多数学函数。