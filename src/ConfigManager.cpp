#include <iostream>
#include "ConfigManager.hpp"

ConfigManager *ConfigManager::self_ = nullptr;
std::mutex ConfigManager::singleton_lock_;

ConfigManager *ConfigManager::GetInstance() {
    std::lock_guard<std::mutex> lock(singleton_lock_);

    if (self_ == nullptr)
        self_ = new ConfigManager();

    return self_;
}

ConfigManager *ConfigManager::GetInstance(std::string path) {
    std::lock_guard<std::mutex> lock(singleton_lock_);

    if (self_ == nullptr)
        self_ = new ConfigManager(path);
    else if(self_->path_ != path) {
        self_->path_ = path;
        self_->Load();
    }

    return self_;
}

void ConfigManager::SetInstance(std::string path) {
    GetInstance(path);
}

ConfigManager::ConfigManager() : ConfigManager("config.json") {}

ConfigManager::ConfigManager(std::string path) {
    path_ = path;
    Load();
}

ConfigManager::~ConfigManager() {
    free(self_);
}

void ConfigManager::Load() {
    std::ifstream in(path_);
    in >> config;
    in.close();
}

const void ConfigManager::Print(int spaces) {
    std::lock_guard<std::mutex> lock(ConfigManager::singleton_lock_);
    std::cout << config.dump(spaces) << std::endl;
}

nlohmann::json ConfigManager::Get(const std::string &key) {
    std::lock_guard<std::mutex> lock(singleton_lock_);
    return config[key];
}

nlohmann::json ConfigManager::IGet(const std::string &key) {
    return GetInstance()->Get(key);
}
