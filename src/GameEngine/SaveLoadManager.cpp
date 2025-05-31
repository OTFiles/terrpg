// src/GameEngine/SaveLoadManager.cpp
#include "SaveLoadManager.h"
#include "GameEngine.h"
#include "Log.h"
#include <regex>

using namespace std;

void SaveLoadManager::saveState(const GameEngine& engine, const std::string& filename) {
    ofstream file(filename, ios::trunc);
    if (!file.is_open()) {
        engine.getDialogSystem().showDialog({
            {"无法创建存档文件"},
            "系统"
        });
        return;
    }

    try {
        // 保存元数据
        file << "// Game Engine Save Format v1.0\n";
        file << "init {\n";

        // 保存玩家状态
        file << "  player " << engine.playerX << " " << engine.playerY << " " << engine.playerDir << "\n";

        // 保存变量
        for (const auto& [key, value] : engine.getVariables()) {
            file << "  var " << key << " = " << value << "\n";
        }

        // 保存访问标记
        for (const auto& marker : engine.getVisitedMarkers()) {
            file << "  marker " << escapeString(marker) << "\n";
        }

        // 保存物品栏
        const auto& inventory = engine.getInventoryManager().getItems();
        for (const auto& item : inventory) {
            file << "  item ";
            serializeGameObject(file, item);
            file << "\n";
        }

        // 保存地图状态
        for (const auto& [mapName, gameMap] : engine.getMaps()) {
            file << "  map " << escapeString(mapName) << " {\n";
            for (const auto& [pos, obj] : gameMap.getAllObjects()) {
                file << "    object " << pos.first << " " << pos.second << " ";
                serializeGameObject(file, obj);
                file << "\n";
            }
            file << "  }\n";
        }

        file << "}\n";
    } catch (const exception& e) {
        Log log("error.log");
        log.error("保存失败: ", string(e.what()));
        engine.getDialogSystem().showDialog({
            {"保存失败: " + string(e.what())},
            "系统"
        });
    }
}

void SaveLoadManager::loadState(GameEngine& engine, const std::string& filename) {
    ifstream file(filename);
    Log log("error.log");

    if (!file.is_open()) {
        engine.getDialogSystem().showDialog({
            {"无法读取存档文件"},
            "系统"
        });
        return;
    }

    try {
        // 重置游戏状态
        engine.getVariables().clear();
        engine.getVisitedMarkers().clear();
        engine.getInventoryManager().clear();
        engine.getMaps().clear();

        string line;
        while (getline(file, line)) {
            size_t commentPos = line.find("//");
            if (commentPos != string::npos) line = line.substr(0, commentPos);
            
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            if (line.empty()) continue;

            if (line == "init {") {
                while (getline(file, line)) {
                    line.erase(0, line.find_first_not_of(" \t"));
                    if (line == "}") break;

                    vector<string> tokens = engine.tokenize(line);
                    if (tokens.empty()) continue;

                    if (tokens[0] == "player") {
                        engine.playerX = stoi(tokens[1]);
                        engine.playerY = stoi(tokens[2]);
                        engine.playerDir = tokens[3][0];
                    }
                    else if (tokens[0] == "var") {
                        engine.getVariables()[tokens[1]] = stoi(tokens[3]);
                    }
                    else if (tokens[0] == "marker") {
                        engine.getVisitedMarkers().insert(unescapeString(tokens[1]));
                    }
                    else if (tokens[0] == "item") {
                        istringstream iss(line.substr(line.find("item") + 4));
                        engine.getInventoryManager().addItem(deserializeGameObject(iss));
                    }
                    else if (tokens[0] == "map") {
                        string mapName = unescapeString(tokens[1]);
                        GameMap& newMap = engine.getMaps()[mapName];
                        
                        while (getline(file, line)) {
                            line.erase(0, line.find_first_not_of(" \t"));
                            if (line == "}") break;
                            
                            vector<string> objTokens = engine.tokenize(line);
                            if (objTokens[0] == "object") {
                                int x = stoi(objTokens[1]);
                                int y = stoi(objTokens[2]);
                                istringstream objIss(line.substr(line.find("object") + 6));
                                newMap.setObject(x, y, deserializeGameObject(objIss));
                            }
                        }
                    }
                }
            }
        }
    } catch (const exception& e) {
        Log log("error.log");
        log.error("读取存档失败: ", string(e.what()));
        engine.getDialogSystem().showDialog({
            {"存档损坏: " + string(e.what())},
            "系统"
        });
    }
}

void SaveLoadManager::serializeGameObject(ostream& os, const GameObject& obj) {
    os << escapeString(obj.name) << " " 
       << escapeString(obj.type) << " "
       << obj.display << " "
       << obj.x << " " << obj.y << " ";

    // 序列化属性
    os << "{";
    for (const auto& [key, value] : obj.properties) {
        os << escapeString(key) << ":";
        visit([&](auto&& arg) { os << arg; }, value);
        os << ";";
    }
    os << "} ";
}

GameObject SaveLoadManager::deserializeGameObject(istream& is) {
    GameObject obj;
    string name, type;
    char display;
    int x, y;
    
    is >> name >> type >> display >> x >> y;
    obj.name = unescapeString(name);
    obj.type = unescapeString(type);
    obj.display = display;
    obj.x = x;
    obj.y = y;

    // 解析属性
    string propBlock;
    is >> propBlock;
    if (propBlock.front() == '{') {
        stringstream props(propBlock.substr(1, propBlock.size()-2));
        string pair;
        while (getline(props, pair, ';')) {
            size_t colon = pair.find(':');
            if (colon != string::npos) {
                string key = unescapeString(pair.substr(0, colon));
                string valueStr = pair.substr(colon+1);
                
                try {
                    if (valueStr.find('.') != string::npos) {
                        obj.properties[key] = stof(valueStr);
                    } else {
                        obj.properties[key] = stoi(valueStr);
                    }
                } catch (...) {
                    obj.properties[key] = valueStr;
                }
            }
        }
    }
    return obj;
}

string SaveLoadManager::escapeString(const string& str) {
    ostringstream oss;
    for (char c : str) {
        switch (c) {
            case ' ': oss << "\\s"; break;
            case '\n': oss << "\\n"; break;
            case '\\': oss << "\\\\"; break;
            default: oss << c;
        }
    }
    return oss.str();
}

string SaveLoadManager::unescapeString(const string& str) {
    string result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\\') {
            if (i+1 < str.size()) {
                switch (str[++i]) {
                    case 's': result += ' '; break;
                    case 'n': result += '\n'; break;
                    case '\\': result += '\\'; break;
                }
            }
        } else {
            result += str[i];
        }
    }
    return result;
}