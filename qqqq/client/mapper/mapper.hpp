#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

//class mapping_parser {
//private:
//    std::unordered_map<std::string, std::string> class_map_;
//    std::unordered_map<std::string, std::string> obf_class_map_;
//    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> field_map_;
//    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> method_map_;
//    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> method_name_map_;
//
//    static void trim(std::string& s);
//    static std::vector<std::string> split(const std::string& s, char delimiter, int max_split = -1);
//
//public:
//    void load(const std::string& file_path);
//
//    std::string get_original_class_name(const std::string& obf_class) const;
//    std::string get_class_mapping(const std::string& original_class) const;
//    std::string get_field_mapping(const std::string& original_class, const std::string& original_field) const;
//    std::string get_method_mapping(const std::string& original_class, const std::string& method_name, const std::vector<std::string>& params) const;
//    std::string get_method_mapping_by_name(const std::string& original_class, const std::string& method_name) const;
//};

class mapping_parser {
public:
    void load(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        std::string current_class;
        std::string line;

        while (std::getline(file, line)) {
            std::string trimmed_line = trim_leading_whitespace(line);

            if (trimmed_line.empty()) continue;

            if (trimmed_line[0] == '#') {
                continue;
            }

            if (trimmed_line.find(" -> ") != std::string::npos && trimmed_line.back() == ':') {
                size_t arrow_pos = trimmed_line.find(" -> ");
                std::string original_class = trimmed_line.substr(0, arrow_pos);
                std::string obfuscated_class = trimmed_line.substr(arrow_pos + 4, trimmed_line.size() - arrow_pos - 5);
                class_map[original_class] = obfuscated_class;
                obf_class_map[obfuscated_class] = original_class;
                current_class = original_class;
                continue;
            }

            size_t arrow_pos = trimmed_line.find(" -> ");
            if (arrow_pos == std::string::npos) {
                continue;
            }

            std::string left_part = trimmed_line.substr(0, arrow_pos);
            std::string obfuscated_name = trimmed_line.substr(arrow_pos + 4);

            if (current_class.empty()) {
                if (isdigit(trimmed_line[0])) {
                    std::vector<std::string> parts = split(left_part, ':');
                    if (parts.size() < 3) continue;
                    std::string method_signature = parts[2];
                    size_t param_start = method_signature.find('(');
                    if (param_start == std::string::npos) continue;
                    std::string before_params = method_signature.substr(0, param_start);
                    std::vector<std::string> sig_parts = split(before_params, ' ');
                    if (sig_parts.size() < 2) continue;
                    std::string return_type = sig_parts[0];
                    std::string method_name = sig_parts[1];

                    size_t array_pos = return_type.find('[');
                    if (array_pos != std::string::npos) {
                        return_type = return_type.substr(0, array_pos);
                    }
                    std::string class_name = return_type;

                    method_map[class_name][method_name] = obfuscated_name;
                }
                else {
                    std::vector<std::string> field_parts = split(left_part, ' ');
                    if (field_parts.size() < 2) continue;
                    std::string class_name = field_parts[0];
                    std::string field_name = field_parts[1];
                    field_map[class_name][field_name] = obfuscated_name;
                }
            }
            else {
                if (isdigit(trimmed_line[0])) {
                    std::vector<std::string> parts = split(left_part, ':');
                    if (parts.size() < 3) continue;
                    std::string method_signature = parts[2];
                    size_t param_start = method_signature.find('(');
                    if (param_start == std::string::npos) continue;
                    std::string before_params = method_signature.substr(0, param_start);
                    std::vector<std::string> sig_parts = split(before_params, ' ');
                    if (sig_parts.empty()) continue;
                    std::string method_name = sig_parts.back();
                    method_map[current_class][method_name] = obfuscated_name;
                }
                else {
                    std::vector<std::string> field_parts = split(left_part, ' ');
                    if (field_parts.empty()) continue;
                    std::string field_name = field_parts.back();
                    field_map[current_class][field_name] = obfuscated_name;
                }
            }
        }
        //value

        field_map["net.minecraft.client.Minecraft"]["screen"] = "y";
        field_map["net.minecraft.client.Minecraft"]["player"] = "s";
        field_map["net.minecraft.client.Minecraft"]["options"] = "m";

        //isDown
        field_map["net.minecraft.client.Options"]["gamma"] = "ce";
        field_map["net.minecraft.client.Options"]["keySprint"] = "C";

        //
        field_map["net.minecraft.client.KeyMapping"]["isDown"] = "p";
        field_map["net.minecraft.client.KeyMapping"]["setDown"] = "a";

        field_map["net.minecraft.client.OptionInstance"]["value"] = "k";

        method_map["net.minecraft.world.phys.HitResult"]["getType"] = "c";

        method_map["net.minecraft.world.item.ItemStack"]["getItem"] = "g";

        method_map["net.minecraft.world.entity.LivingEntity"]["getMainHandItem"] = "eT";

        //s
    }

    std::string get_class_mapping(const std::string& original_class) const
    {
        auto it = class_map.find(original_class);
        return it != class_map.end() ? it->second : "";
    }

    std::string get_original_class_name(const std::string& obf_class) const
    {
        auto it = obf_class_map.find(obf_class);
        return it != obf_class_map.end() ? it->second : obf_class;
    }

    std::string get_field_mapping(const std::string& original_class, const std::string& original_field) const 
    {
        auto class_it = field_map.find(original_class);
        if (class_it == field_map.end()) return "";
        auto field_it = class_it->second.find(original_field);
        return field_it != class_it->second.end() ? field_it->second : "";
    }

    std::string get_method_mapping(const std::string& original_class, const std::string& original_method) const 
    {
        auto class_it = method_map.find(original_class);
        if (class_it == method_map.end()) return "";
        auto method_it = class_it->second.find(original_method);


        return method_it != class_it->second.end() ? method_it->second : "";
    }

private:
    std::unordered_map<std::string, std::string> obf_class_map;
    std::unordered_map<std::string, std::string> class_map;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> field_map;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> method_map;

    static std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }
        return tokens;
    }

    static std::string trim_leading_whitespace(const std::string& s) {
        size_t start = s.find_first_not_of(" \t");
        return (start == std::string::npos) ? "" : s.substr(start);
    }
};

inline extern std::unique_ptr<::mapping_parser> mapper = std::make_unique<::mapping_parser>();