#include "pch.hpp"
#include "mapper.hpp"

//void mapping_parser::trim(std::string& s) {
//    size_t start = s.find_first_not_of(" \t");
//    size_t end = s.find_last_not_of(" \t");
//    if (start == std::string::npos)
//        s = "";
//    else
//        s = s.substr(start, end - start + 1);
//}
//
//std::vector<std::string> mapping_parser::split(const std::string& s, char delimiter, int max_split) {
//    std::vector<std::string> tokens;
//    std::string token;
//    std::istringstream tokenStream(s);
//    int count = 0;
//    while (std::getline(tokenStream, token, delimiter)) {
//        if (max_split != -1 && count >= max_split) {
//            if (tokenStream.rdbuf()->in_avail() != 0) {
//                std::string rest;
//                std::getline(tokenStream, rest);
//                token += delimiter + rest;
//            }
//            tokens.push_back(token);
//            break;
//        }
//        tokens.push_back(token);
//        count++;
//    }
//    return tokens;
//}
//
//void mapping_parser::load(const std::string& file_path) {
//    std::ifstream file(file_path);
//    if (!file.is_open()) {
//        return;
//    }
//
//    std::string current_class_original;
//    std::string line;
//
//    while (std::getline(file, line)) {
//        trim(line);
//        if (line.empty() || line[0] == '#') {
//            continue;
//        }
//
//        if (line.find(" -> ") != std::string::npos && line.back() == ':') {
//            size_t arrow_pos = line.find(" -> ");
//            std::string original_class = line.substr(0, arrow_pos);
//            std::string obfuscated_class = line.substr(arrow_pos + 4, line.size() - arrow_pos - 5);
//            trim(original_class);
//            trim(obfuscated_class);
//
//            class_map_[original_class] = obfuscated_class;
//            obf_class_map_[obfuscated_class] = original_class;
//            current_class_original = original_class;
//        }
//        else {
//            if (current_class_original.empty()) {
//                continue;
//            }
//
//            if (!line.empty() && std::isdigit(line[0])) {
//                std::vector<std::string> parts = split(line, ':', 2);
//                if (parts.size() < 3) {
//                    continue;
//                }
//                std::string rest = parts[2];
//                size_t arrow_pos = rest.find(" -> ");
//                if (arrow_pos == std::string::npos) {
//                    continue;
//                }
//                std::string left_part = rest.substr(0, arrow_pos);
//                std::string obfuscated = rest.substr(arrow_pos + 4);
//                trim(left_part);
//                trim(obfuscated);
//
//                size_t paren_open = left_part.find('(');
//                if (paren_open == std::string::npos) {
//                    continue;
//                }
//
//                std::string params_part = left_part.substr(paren_open + 1);
//                size_t paren_close = params_part.find(')');
//                if (paren_close == std::string::npos) {
//                    continue;
//                }
//                std::string params_str = params_part.substr(0, paren_close);
//                params_str.erase(std::remove(params_str.begin(), params_str.end(), ' '), params_str.end());
//
//                std::string method_part = left_part.substr(0, paren_open);
//                size_t last_space = method_part.rfind(' ');
//                if (last_space == std::string::npos) {
//                    continue;
//                }
//
//                std::string return_type = method_part.substr(0, last_space);
//                std::string method_name = method_part.substr(last_space + 1);
//
//                std::string signature = method_name + "(" + params_str + ")";
//                method_map_[current_class_original][signature] = obfuscated;
//                method_name_map_[current_class_original][method_name] = obfuscated;
//            }
//            else {
//                size_t arrow_pos = line.find(" -> ");
//                if (arrow_pos == std::string::npos) {
//                    continue;
//                }
//                std::string left_part = line.substr(0, arrow_pos);
//                std::string obfuscated = line.substr(arrow_pos + 4);
//                trim(left_part);
//                trim(obfuscated);
//
//                size_t space_pos = left_part.find(' ');
//                if (space_pos == std::string::npos) {
//                    continue;
//                }
//                std::string original_class_part = left_part.substr(0, space_pos);
//                std::string field_name = left_part.substr(space_pos + 1);
//                trim(original_class_part);
//                trim(field_name);
//
//                if (original_class_part != current_class_original) {
//                    continue;
//                }
//                std::cout << "current_class_original << " << current_class_original << " field_name << " << field_name << std::endl;
//                field_map_[current_class_original][field_name] = obfuscated;
//            }
//        }
//    }
//}
//
//std::string mapping_parser::get_original_class_name(const std::string& obf_class) const {
//    auto it = obf_class_map_.find(obf_class);
//    return it != obf_class_map_.end() ? it->second : obf_class;
//}
//
//std::string mapping_parser::get_class_mapping(const std::string& original_class) const {
//    auto it = class_map_.find(original_class);
//    return it != class_map_.end() ? it->second : "";
//}
//
//std::string mapping_parser::get_field_mapping(const std::string& original_class, const std::string& original_field) const {
//    auto class_it = field_map_.find(original_class);
//    if (class_it == field_map_.end()) {
//        return "";
//    }
//
//    auto field_it = class_it->second.find(original_field);
//    return field_it != class_it->second.end() ? field_it->second : "";
//}
//
//std::string mapping_parser::get_method_mapping(const std::string& original_class, const std::string& method_name, const std::vector<std::string>& params) const {
//    auto class_it = method_map_.find(original_class);
//    if (class_it == method_map_.end()) {
//        return "";
//    }
//
//    std::string params_str;
//    for (size_t i = 0; i < params.size(); ++i) {
//        if (i > 0) {
//            params_str += ",";
//        }
//        params_str += params[i];
//    }
//
//    std::string signature = method_name + "(" + params_str + ")";
//    auto method_it = class_it->second.find(signature);
//    return method_it != class_it->second.end() ? method_it->second : "";
//}
//
//std::string mapping_parser::get_method_mapping_by_name(const std::string& original_class, const std::string& method_name) const {
//    auto class_it = method_name_map_.find(original_class);
//
//    if (class_it == method_name_map_.end()) {
//        return "";
//    }
//
//    auto method_it = class_it->second.find(method_name);
//    return method_it != class_it->second.end() ? method_it->second : "";
//}
