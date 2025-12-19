//
// Created by WIN10 on 2024/8/22.
//
#include <iostream>
#include <string>
#include <map>
#include <windows.h>

#include "ini.h"

bool FileExists(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES &&
            !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

std::string GetExecutablePath() {
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}

static int custom_ini_handler(void* user, const char* section, const char* name, const char* value) {
    auto* config = (std::map<std::string, std::string>*)user;
    std::string key = std::string(section) + "." + name;
    (*config)[key] = value;
    return 1;
}

std::map<std::string, std::string> GetConfig() {
    std::string current_path = GetExecutablePath();
    std::map<std::string, std::string> config;

    std::string ini_path = current_path + "\\conf.ini";
    std::string txt_path = current_path + "\\conf.ini.txt";

    // 1. ini 优先
    if (FileExists(ini_path)) {
        std::cout << "Load config: " << ini_path << std::endl;
        if (ini_parse(ini_path.c_str(), custom_ini_handler, &config) >= 0) {
            return config;
        }
        std::cout << "Failed to parse conf.ini" << std::endl;
    }

    // 2. fallback 到 txt
    if (FileExists(txt_path)) {
        std::cout << "Load config: " << txt_path << std::endl;
        if (ini_parse(txt_path.c_str(), custom_ini_handler, &config) >= 0) {
            return config;
        }
        std::cout << "Failed to parse conf.txt" << std::endl;
    }

    std::cout << current_path << " Can't find 'conf.ini' or 'conf.txt'\n";
    return {};
}
