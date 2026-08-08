// PresetManager.h — .fst preset load/save header

#pragma once
#include <string>
#include <vector>

struct Preset {
    std::string name;
    std::string filePath;
    std::string pluginName;
};

class PresetManager {
public:
    std::vector<Preset> loadAll(const std::string& pluginName);
    bool save(const Preset& preset, const std::vector<uint8_t>& data);
    bool remove(const Preset& preset);
};
