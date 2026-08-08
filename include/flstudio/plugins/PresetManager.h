#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace fl {

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

private:
    static std::filesystem::path presetRoot();
};

} // namespace fl