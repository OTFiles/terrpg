// File: src/GameEngine/Commands/ConcreteCommands/ItemCommand.cpp
#include "ItemCommand.h"
#include "../CommandUtils.h"
#include <algorithm>
#include <sstream>
#include <iostream>

void ItemCommand::handle(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 2) throw std::runtime_error("Usage: /item <define|setproperty|...>");
    
    const std::string& subcmd = args[1];
    if (subcmd == "define") handleDefine(args, engine);
    else if (subcmd == "setproperty") handleSetProperty(args, engine);
    else if (subcmd == "give") handleGive(args, engine);
    else throw std::runtime_error("未知子命令: " + subcmd);
}

void ItemCommand::handleDefine(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 3) throw std::runtime_error("Usage: /item define <name> [type=generic]");
    
    std::string name = args[2];
    auto params = CommandUtils::parseNamedParams(args, 3);
    
    GameObject item;
    item.type = "item";
    item.name = name;
    item.display = params.count("display") ? params["display"][0] : '$';
    
    // 设置默认属性
    static const std::unordered_map<std::string, int> DEFAULT_PROPS = {
        {"pickupable", 0}, {"stackable", 1}, {"value", 10}
    };
    for (const auto& [k, v] : DEFAULT_PROPS) {
        if (!params.count(k)) item.setProperty(k, v);
    }
    
    // 应用自定义属性
    for (const auto& [k, v] : params) {
        if (k != "name" && k != "display") {
            item.setProperty(k, v);
        }
    }
    
    engine.getItems()[name] = item;
    std::string itemType = params.count("type") ? params.at("type") : "generic";
    engine.showDialog("系统", "物品 " + name + " 定义成功 (类型: " + itemType + ")");
    
#ifdef DEBUG
    // 定义调试用的属性获取函数
    auto getPropString = [&](const std::string& prop) -> std::string {
        if (!item.hasProperty(prop)) return "未设置";
        
        try {
            if (item.getProperty<int>(prop, -1) != -1) 
                return std::to_string(item.getProperty<int>(prop));
            if (item.getProperty<bool>(prop, false)) 
                return item.getProperty<bool>(prop) ? "true" : "false";
            return item.getProperty<std::string>(prop);
        } catch (...) {
            return "类型错误";
        }
    };
    
    std::clog << "[DEBUG] ItemCommand::handleDefine" << "\n";
    std::clog << " - Command: /item define " << name << "\n";
    std::clog << " - Item Name: " << name << "\n";
    std::clog << " - Type: " << itemType << "\n";
    std::clog << " - Display: " << item.display << "\n";
    std::clog << " - Pickupable: " << getPropString("pickupable") << "\n";
    std::clog << " - Stackable: " << getPropString("stackable") << "\n";
    std::clog << " - Value: " << getPropString("value") << std::endl;
#endif
}

void ItemCommand::handleSetProperty(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 4) throw std::runtime_error("Usage: /item setproperty <name> <property=value>...");
    
    std::string name = args[2];
    auto params = CommandUtils::parseNamedParams(args, 3);
    
    if (!engine.getItems().count(name)) {
        throw std::runtime_error("未定义的物品: " + name);
    }
    
    GameObject& item = engine.getItems()[name];
    
#ifdef DEBUG
    auto getPropString = [&](const std::string& prop) -> std::string {
        if (!item.hasProperty(prop)) return "未设置";
        
        try {
            if (item.getProperty<int>(prop, -1) != -1) 
                return std::to_string(item.getProperty<int>(prop));
            if (item.getProperty<bool>(prop, false)) 
                return item.getProperty<bool>(prop) ? "true" : "false";
            return item.getProperty<std::string>(prop);
        } catch (...) {
            return "类型错误";
        }
    };
    std::clog << "[DEBUG] ItemCommand::handleSetProperty" << "\n";
    std::clog << " - Command: /item setproperty " << name << "\n";
#endif
    
    for (const auto& [prop, value] : params) {
#ifdef DEBUG
        std::string oldValue = getPropString(prop);
#endif
        // 处理特殊属性类型
        if (prop == "damage") {
            item.setProperty("damage", std::to_string(std::stoi(value)));
        } else if (prop == "pickupable") {
            bool state = (value == "true" || value == "1" || value == "是");
            item.setProperty("pickupable", state ? 1 : 0);
        } else if (prop == "stackable" || prop == "value") {
            item.setProperty(prop, value);
        }
#ifdef DEBUG
        std::clog << " - 属性变更: " << prop << " | 旧值: " << oldValue << " → 新值: " << getPropString(prop) << "\n";
#endif
    }
    
    engine.showDialog("系统", "已更新物品属性: " + name + "\n新属性: " + item.getFormattedProperties());
}

void ItemCommand::handleGive(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 3) throw std::runtime_error("Usage: /item give <item> [amount=1]");

    const std::string& itemName = args[2];
    int amount = 1;
    
    // 解析数量参数
    auto params = CommandUtils::parseNamedParams(args, 3);
    if(params.count("amount")) {
        amount = std::stoi(params["amount"]);
    } else if(args.size() >= 4) {
        amount = std::stoi(args[3]); // 兼容旧版位置参数
    }
    
    // 检查物品是否存在
    if (!engine.getItems().count(itemName)) {
        throw std::runtime_error("未定义的物品: " + itemName);
    }
    
    // 获取物品模板
    const GameObject& itemTemplate = engine.getItems().at(itemName);
    
    // 堆叠逻辑处理
    const bool stackable = itemTemplate.getProperty<int>("stackable", 0);
    if (stackable) {
        // 寻找可堆叠的现有物品
        for (auto& existingItem : engine.getInventory()) {
            if (existingItem.name == itemName) {
                int currentCount = existingItem.getProperty<int>("count", 1);
                existingItem.setProperty("count", currentCount + amount);
                engine.showDialog("系统", "成功给予 " + std::to_string(amount) + " 个 " + itemName);
                return;
            }
        }
    }
    // 添加新物品实例
    GameObject newItem = itemTemplate;
    newItem.setProperty("count", amount);
    newItem.setProperty("instance_id", engine.generateItemInstanceId());
    engine.getInventory().push_back(newItem);
    
    engine.showDialog("系统", "成功给予 " + std::to_string(amount) + " 个 " + itemName);
}