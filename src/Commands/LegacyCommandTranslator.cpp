#include <vector>
#include <string>

class LegacyCommandTranslator {
public:
    static std::string translate(const std::vector<std::string>& tokens) {
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
};