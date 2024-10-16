//
// Created by WIN10 on 2024/8/22.
//
#include <iostream>
#include <string>
#include <map>
#include <windows.h>

#include "ini.h"

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
    //std::cout << current_path << std::endl;

    std::map<std::string, std::string> config;
    current_path = "C:\\Users\\WIN10\\Desktop";
    std::string config_file_path = current_path + "\\conf.ini";
    std::cout << config_file_path << std::endl;
    if (ini_parse(config_file_path.c_str(), custom_ini_handler, &config) < 0) {
        std::cout << "Can't load 'conf.ini'\n";
        return {};
    }
    return config;
}