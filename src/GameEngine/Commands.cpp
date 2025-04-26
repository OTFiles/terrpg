// File: src/GameEngine/Commands.cpp
#include "GameEngine.h"
#include <unordered_map>
#include <functional>
#include <sstream>
#include <algorithm>

class CommandParser {
private:
    GameEngine& engine;
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> commands;
    
    void registerCommands() {
        commands["/map"] = [this](const auto& args) { handleMapCommand(args); };
        commands["/npc"] = [this](const auto& args) { handleNpcCommand(args); };
        commands["/item"] = [this](const auto& args) { handleItemCommand(args); };
        commands["/entity"] = [this](const auto& args) { handleEntityCommand(args); };
        commands["/teleport"] = [this](const auto& args) { handleTeleportCommand(args); };
        commands["/trigger"] = [this](const auto& args) { handleTriggerCommand(args); };
        commands["/scoreboard"] = [this](const auto& args) { handleScoreboardCommand(args); };
    }

    // 解析命名参数 (key=value)
    std::unordered_map<std::string, std::string> parseNamedParams(const std::vector<std::string>& args, size_t start = 0) {
        std::unordered_map<std::string, std::string> params;
        for (size_t i = start; i < args.size(); ++i) {
            size_t eqPos = args[i].find('=');
            if (eqPos != std::string::npos) {
                std::string key = args[i].substr(0, eqPos);
                std::string value = args[i].substr(eqPos + 1);
                params[key] = value;
            }
        }
        return params;
    }

    // 解析坐标 (x,y)
    std::pair<int, int> parseCoordinates(const std::vector<std::string>& args, size_t index) {
        try {
            // 尝试x,y格式
            if (args[index].find(',') != std::string::npos) {
                std::vector<std::string> parts;
                std::stringstream ss(args[index]);
                std::string part;
                while (std::getline(ss, part, ',')) {
                    parts.push_back(part);
                }
                if (parts.size() >= 2) {
                    return {std::stoi(parts[0]), std::stoi(parts[1])};
                }
            }
            // 尝试x y格式
            if (index + 1 < args.size()) {
                return {std::stoi(args[index]), std::stoi(args[index+1])};
            }
        } catch (...) {
            throw std::runtime_error("Invalid coordinates format");
        }
        throw std::runtime_error("Not enough coordinates provided");
    }

public:
    CommandParser(GameEngine& eng) : engine(eng) {
        registerCommands();
    }

    void executeCommand(const std::string& commandLine) {
        std::vector<std::string> tokens;
        std::stringstream ss(commandLine);
        std::string token;
        
        while (ss >> token) {
            tokens.push_back(token);
        }
        
        if (tokens.empty()) return;
        
        try {
            auto it = commands.find(tokens[0]);
            if (it != commands.end()) {
                it->second(tokens);
            } else {
                engine.showDialog("系统", "未知命令: " + tokens[0]);
            }
        } catch (const std::exception& e) {
            engine.showDialog("错误", std::string("命令执行失败: ") + e.what());
        }
    }

private:
    // 命令处理函数
    void handleMapCommand(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            throw std::runtime_error("Usage: /map <create|setblock|fill|...>");
        }
        
        const std::string& subcmd = args[1];
        
        if (subcmd == "create") {
            if (args.size() < 3) throw std::runtime_error("Usage: /map create <name> [width=20] [height=20]");
            
            std::string name = args[2];
            int width = 20, height = 20;
            auto params = parseNamedParams(args, 3);
            
            if (params.count("width")) width = std::stoi(params["width"]);
            if (params.count("height")) height = std::stoi(params["height"]);
            
            engine.getMaps()[name] = GameMap(width, height);
            engine.showDialog("系统", "地图 " + name + " 创建成功");
            
        } else if (subcmd == "setblock") {
            if (args.size() < 6) throw std::runtime_error("Usage: /map setblock <map> <x> <y> <type> [options...]");
            
            std::string mapName = args[2];
            auto [x, y] = parseCoordinates(args, 3);
            std::string type = args[5];
            auto params = parseNamedParams(args, 6);
            
            GameObject obj;
            // 如果是item类型，从物品库获取模板
            if (type == "item") {
                if (!engine.getItems().count(params["name"])) {
                    throw std::runtime_error("未定义的物品: " + params["name"]);
                }
                obj = engine.getItems()[params["name"]];
            } else {
                obj.type = type;
            }
            obj.x = x;
            obj.y = y;
            
            // 设置名称
            if (params.count("name")) {
                obj.name = params["name"];
            }
            
            // 设置显示字符
            if (params.count("display")) {
                obj.display = params["display"][0];
            } else {
                // 默认显示字符
                static std::unordered_map<std::string, char> defaults = {
                    {"wall", '#'}, {"npc", '@'}, {"item", '$'},
                    {"trap", '^'}, {"marker", '*'}
                };
                obj.display = defaults.count(type) ? defaults[type] : '?';
            }
            
            // 设置属性
            if (type == "wall") {
                obj.setProperty("walkable", 0);
            } else if (type == "trap") {
                int damage = params.count("damage") ? std::stoi(params["damage"]) : 10;
                obj.setProperty("damage", damage);
                obj.setProperty("walkable", 1);
            }
            
            engine.getMaps()[mapName].setObject(x, y, obj);
            
        } else if (subcmd == "fill") {
            if (args.size() < 7) throw std::runtime_error("Usage: /map fill <map> <from x y> <to x y> <type> [options...]");
            
            std::string mapName = args[2];
            auto [x1, y1] = parseCoordinates(args, 3);
            auto [x2, y2] = parseCoordinates(args, 4);
            std::string type = args[5];
            auto params = parseNamedParams(args, 6);
            
            GameObject obj;
            // 如果是item类型，从物品库获取模板
            if (type == "item") {
                if (!engine.getItems().count(params["name"])) {
                    throw std::runtime_error("未定义的物品: " + params["name"]);
                }
                obj = engine.getItems()[params["name"]];
            } else {
                obj.type = type;
            }
            obj.x = x1;
            obj.y = y1;
            
            // 设置名称
            if (params.count("name")) {
                obj.name = params["name"];
            }
            
            // 设置显示字符
            if (params.count("display")) {
                obj.display = params["display"][0];
            } else {
                // 默认显示字符
                static std::unordered_map<std::string, char> defaults = {
                    {"wall", '#'}, {"trap", '^'}
                };
                obj.display = defaults.count(type) ? defaults[type] : '?';
            }
            
            // 设置属性
            if (type == "wall") {
                obj.setProperty("walkable", 0);
            } else if (type == "trap") {
                int damage = params.count("damage") ? std::stoi(params["damage"]) : 10;
                obj.setProperty("damage", damage);
                obj.setProperty("walkable", 1);
            }
            
            // 填充区域
            for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
                for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y) {
                    engine.getMaps()[mapName].setObject(x, y, obj);
                }
            }
            
            engine.showDialog("系统", "填充操作完成");
        } else {
            throw std::runtime_error("未知子命令: " + subcmd);
        }
    }
    
    void handleNpcCommand(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            throw std::runtime_error("Usage: /npc <create|setdialogue|...>");
        }
        
        const std::string& subcmd = args[1];
        
        if (subcmd == "create") {
            if (args.size() < 3) throw std::runtime_error("Usage: /npc create <name> [template=default]");
            
            std::string name = args[2];
            auto params = parseNamedParams(args, 3);
            
            GameObject npc;
            npc.type = "npc";
            npc.name = name;
            npc.display = '@';
            
            engine.getNpcs()[name] = npc;
            engine.showDialog("系统", "NPC " + name + " 创建成功");
            
        } else if (subcmd == "setdialogue") {
            if (args.size() < 5) throw std::runtime_error("Usage: /npc setdialogue <name> <condition> <dialogue>");
            
            std::string name = args[2];
            std::string condition = args[3];
            std::string dialogue;
            
            // 合并剩余参数作为对话内容
            for (size_t i = 4; i < args.size(); ++i) {
                if (i > 4) dialogue += " ";
                dialogue += args[i];
            }
            
            engine.getNpcs()[name].dialogues[condition] = dialogue;
            engine.showDialog("系统", "已为NPC " + name + " 设置对话");
        } else {
            throw std::runtime_error("未知子命令: " + subcmd);
        }
    }
    
    void handleItemCommand(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            throw std::runtime_error("Usage: /item <define|setproperty|...>");
        }
        
        const std::string& subcmd = args[1];
        
        if (subcmd == "define") {
            if (args.size() < 3) throw std::runtime_error("Usage: /item define <name> [type=generic]");
            
            std::string name = args[2];
            auto params = parseNamedParams(args, 3);
            
            GameObject item;
            item.type = "item";
            item.name = name;
            item.display = '$';
            
            engine.getItems() [name] = item;
            engine.showDialog("系统", "物品 " + name + " 定义成功");
            
        } else if (subcmd == "setproperty") {
            if (args.size() < 4) throw std::runtime_error("Usage: /item setproperty <name> <property=value>");
            
            std::string name = args[2];
            auto params = parseNamedParams(args, 3);
            
            for (const auto& [prop, value] : params) {
                if (prop == "damage") {
                    engine.getItems() [name].setProperty("damage", std::stoi(value));
                } else if (prop == "pickupable") {
                    engine.getItems() [name].setProperty("pickupable", (value == "true" || value == "1" || value == "是") ? 1 : 0);
                } else {
                    engine.getItems() [name].setProperty(prop, value);
                }
            }
            
            engine.showDialog("系统", "已更新物品 " + name + " 的属性");
        } else {
            throw std::runtime_error("未知子命令: " + subcmd);
        }
    }
    
    void handleEntityCommand(const std::vector<std::string>& args) {
        if (args.size() < 3) {
            throw std::runtime_error("Usage: /entity <set|get> <name> <property> [value]");
        }
        
        const std::string& subcmd = args[1];
        const std::string& name = args[2];
        
        if (subcmd == "set") {
            if (args.size() < 5) throw std::runtime_error("Usage: /entity set <name> <property> <value>");
            
            std::string property = args[3];
            std::string value;
            
            // 合并剩余参数作为值
            for (size_t i = 4; i < args.size(); ++i) {
                if (i > 4) value += " ";
                value += args[i];
            }
            
            // 查找实体 (在NPC、物品或地图对象中)
            if (engine.getNpcs().count(name)) {
                engine.getNpcs()[name].setProperty(property, value);
            } else if (engine.getItems() .count(name)) {
                engine.getItems() [name].setProperty(property, value);
            } else {
                // 尝试在地图中查找
                bool found = false;
                for (auto& [mapName, gameMap] : engine.getMaps()) {
                    if (gameMap.hasObject(name)) {
                        gameMap.getObjectByName(name).setProperty(property, value);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    throw std::runtime_error("未找到实体: " + name);
                }
            }
            
            engine.showDialog("系统", "已更新实体 " + name + " 的属性 " + property);
        } else {
            throw std::runtime_error("未知子命令: " + subcmd);
        }
    }
    
    void handleTeleportCommand(const std::vector<std::string>& args) {
        if (args.size() < 4) {
            throw std::runtime_error("Usage: /teleport <map> <x> <y>");
        }
        
        std::string mapName = args[1];
        auto [x, y] = parseCoordinates(args, 2);
        
        if (engine.getMaps().count(mapName)) {
            engine.setCurrentMap(mapName);
            engine.setPlayerX(x);
            engine.setPlayerY(y);
            engine.showDialog("系统", "已传送到 " + mapName + " (" + std::to_string(x) + "," + std::to_string(y) + ")");
        } else {
            throw std::runtime_error("地图不存在: " + mapName);
        }
    }
    
    void handleTriggerCommand(const std::vector<std::string>& args) {
        if (args.size() < 3) {
            throw std::runtime_error("Usage: /trigger <event> [args...]");
        }
        
        const std::string& event = args[1];
        
        if (event == "npc.interact") {
            if (args.size() < 3) throw std::runtime_error("Usage: /trigger npc.interact <name> [condition]");
            
            std::string name = args[2];
            std::string condition = args.size() > 3 ? args[3] : "always";
            
            if (engine.getNpcs().count(name)) {
                if (engine.getNpcs()[name].dialogues.count(condition)) {
                    engine.showDialog(name, engine.getNpcs()[name].dialogues[condition]);
                } else {
                    engine.showDialog(name, "...");
                }
            } else {
                throw std::runtime_error("NPC不存在: " + name);
            }
        } else {
            throw std::runtime_error("未知事件: " + event);
        }
    }
    
    void handleScoreboardCommand(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            throw std::runtime_error("Usage: /scoreboard <add|set|operation>");
        }
        
        const std::string& subcmd = args[1];
        
        if (subcmd == "add") {
            if (args.size() < 3) throw std::runtime_error("Usage: /scoreboard add <variable>");
            engine.getVariables()[args[2]] = 0;
            
        } else if (subcmd == "set") {
            if (args.size() < 4) throw std::runtime_error("Usage: /scoreboard set <variable> <value>");
            engine.getVariables()[args[2]] = engine.evalExpression(args[3]);
            
        } else if (subcmd == "operation") {
            if (args.size() < 5) throw std::runtime_error("Usage: /scoreboard operation <variable> <op> <expression>");
            
            std::string varName = args[2];
            std::string op = args[3];
            std::string expr;
            
            for (size_t i = 4; i < args.size(); ++i) {
                if (i > 4) expr += " ";
                expr += args[i];
            }
            
            int value = engine.evalExpression(expr);
            
            if (op == "=") {
                engine.getVariables()[varName] = value;
            } else if (op == "+=") {
                engine.getVariables()[varName] += value;
            } else if (op == "-=") {
                engine.getVariables()[varName] -= value;
            } else {
                throw std::runtime_error("未知操作符: " + op);
            }
        } else {
            throw std::runtime_error("未知子命令: " + subcmd);
        }
    }
};

std::string translateLegacyCommand(const std::vector<std::string>& tokens) {
    if (tokens.empty()) return "";

    std::stringstream newCmd;
    
    // 转换 add 命令
    if (tokens[0] == "add") {
        if (tokens[1] == "map") {
            newCmd << "/map create " << tokens[2];
        } 
        else if (tokens[1] == "npc") {
            newCmd << "/npc create " << tokens[2];
        }
        else if (tokens[1] == "item") {
            newCmd << "/item define " << tokens[2];
        }
        else if (tokens[1] == "变量") {
            newCmd << "/scoreboard add " << tokens[2];
        }
    }
    // 转换 set 命令
    else if (tokens[0] == "set") {
        if (tokens[1] == "map") {
            newCmd << "/map setblock " << tokens[2] << " " << tokens[3] << " " << tokens[4] << " " << tokens[5];
            if (tokens.size() > 6) newCmd << " name=" << tokens[6];
            if (tokens.size() > 7) newCmd << " display=" << tokens[7] << " type=" << tokens[5];
        }
        else if (tokens[1] == "npc") {
            std::string condition = (tokens.size() > 4) ? tokens[4] : "always";
            newCmd << "/npc setdialogue " << tokens[2] << " " << condition << " \"" << tokens.back() << "\"";
        }
        else if (tokens[1] == "item") {
            if (tokens[3] == "伤害") {
                newCmd << "/item setproperty " << tokens[2] << " damage=" << tokens[4];
            }
            else if (tokens[3] == "可拾取" || tokens[3] == "pickupable") {
                newCmd << "/item setproperty " << tokens[2] << " pickupable=" << tokens[4];
            }
        }
        else if (tokens[1] == "变量") {
            newCmd << "/scoreboard set " << tokens[2] << " " << tokens[4];
        }
    }
    // 转换 fill 命令
    else if (tokens[0] == "fill") {
        newCmd << "/map fill " << tokens[1] << " " << tokens[2] << "," << tokens[3]
               << " " << tokens[4] << "," << tokens[5] << " " << tokens[6];
        if (tokens.size() > 7) newCmd << " name=" << tokens[7] << " display=" << tokens[6][0];
    }
    // 转换 tp 命令
    else if (tokens[0] == "tp") {
        newCmd << "/teleport " << tokens[1] << " " << tokens[2] << " " << tokens[3];
    }
    // 转换 run 命令
    else if (tokens[0] == "run") {
        if (tokens[1] == "npc") {
            newCmd << "/trigger npc.interact " << tokens[2];
            if (tokens.size() > 3) newCmd << " " << tokens[4];
        }
    }
    // 未知命令保持原样
    else {
        for (const auto& t : tokens) newCmd << t << " ";
    }
    return newCmd.str();
}

void GameEngine::runCommand(const std::vector<std::string>& tokens) {
    if (tokens.empty()) return;
    
    // 将旧命令转换为新命令格式
    std::string commandLine;
    for (const auto& token : tokens) {
        if (!commandLine.empty()) commandLine += " ";
        commandLine += token;
    }
    
    // 使用新的命令解析器
    commandLine = translateLegacyCommand(tokens);
CommandParser parser(*this);  // 改为非静态局部变量
    parser.executeCommand(commandLine.empty() ? commandLine : commandLine);
}