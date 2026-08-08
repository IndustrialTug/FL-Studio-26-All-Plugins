#include "flstudio/plugins/PresetManager.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

namespace fl {

fs::path PresetManager::presetRoot()
{
    return fs::path(getenv("APPDATA")) / "Image-Line" / "FL Studio" / "Presets";
}

std::vector<Preset> PresetManager::loadAll(const std::string& pluginName)
{
    std::vector<Preset> out;
    fs::path dir = presetRoot() / pluginName;
    if (!fs::exists(dir)) return out;
    for (auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() != ".fst") continue;
        Preset p;
        p.name       = entry.path().stem().string();
        p.filePath   = entry.path().string();
        p.pluginName = pluginName;
        out.push_back(std::move(p));
    }
    return out;
}

bool PresetManager::save(const Preset& preset, const std::vector<uint8_t>& data)
{
    fs::path dir = presetRoot() / preset.pluginName;
    fs::create_directories(dir);
    fs::path file = dir / (preset.name + ".fst");
    std::ofstream ofs(file, std::ios::binary);
    if (!ofs) return false;
    ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

bool PresetManager::remove(const Preset& preset)
{
    fs::path file = presetRoot() / preset.pluginName / (preset.name + ".fst");
    return fs::remove(file);
}

} // namespace fl