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
        {"pickupable", 1}, {"stackable", 1}, {"value", 10}
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
    
#ifdef DEBUG
    std::clog << "[DEBUG] ItemCommand" << "\n";
    std::clog << " - command: /item define" << "\n";
    std::clog << " - Defining item: " << name << "\n";
    std::clog << " - Display: " << item.display << "\n";
    std::clog << " - Pickupable: " << item.getProperty("pickupable", 0) << "\n";
    std::clog << " - Stackable: " << item.getProperty("stackable", 0) << std::endl;
#endif
    
    engine.getItems()[name] = item;
    std::string itemType = params.count("type") ? params.at("type") : "generic";
    engine.showDialog("系统", "物品 " + name + " 定义成功 (类型: " + itemType + ")");
}

void ItemCommand::handleSetProperty(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 4) throw std::runtime_error("Usage: /item setproperty <name> <property=value>...");
    
    std::string name = args[2];
    auto params = CommandUtils::parseNamedParams(args, 3);
    
    if (!engine.getItems().count(name)) {
        throw std::runtime_error("未定义的物品: " + name);
    }
    
    GameObject& item = engine.getItems()[name];
    for (const auto& [prop, value] : params) {
        // 处理特殊属性类型
        if (prop == "damage") {
            item.setProperty("damage", std::to_string(std::stoi(value)));
        } else if (prop == "pickupable") {
            bool state = (value == "true" || value == "1" || value == "是");
            item.setProperty("pickupable", state ? 1 : 0);
        } else {
            item.setProperty(prop, value);
        }
    }
    
    engine.showDialog("系统", "已更新物品属性: " + name + "\n新属性: " + item.getFormattedProperties());
}