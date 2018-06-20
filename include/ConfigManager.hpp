#ifndef STRAWBERRYDATA_CONFIG_H
#define STRAWBERRYDATA_CONFIG_H

#include <string>
#include <fstream>

#include <json.hpp>
#include <mutex>

class ConfigManager {
public:
    static ConfigManager* GetInstance();
    static ConfigManager* GetInstance(std::string path);
    static void SetInstance(std::string path);
    ConfigManager& operator=(ConfigManager const&) = delete;
    static nlohmann::json IGet(const std::string& key);
    nlohmann::json Get(const std::string& key);
    const void Print(int spaces = 2);
private:
    ConfigManager();
    explicit ConfigManager(std::string path);
    ~ConfigManager();
    void Load();

    static ConfigManager *self_;
    static std::mutex singleton_lock_;

    nlohmann::json config;
    std::string path_;
};


#endif //STRAWBERRYDATA_CONFIG_H
