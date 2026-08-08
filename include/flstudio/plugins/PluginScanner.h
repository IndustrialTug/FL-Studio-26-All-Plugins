#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fl {

enum class PluginFormat { VST2, VST3 };

struct PluginInfo {
    std::string path;
    std::string name;
    PluginFormat format = PluginFormat::VST2;
};

class PluginScanner {
public:
    std::vector<PluginInfo> scan();
    void saveCache(const std::vector<PluginInfo>& plugins, const std::string& cacheFile);
    void addSearchPath(const std::string& path) { m_paths.push_back(path); }

private:
    std::vector<std::filesystem::path> m_paths;
};

} // namespace fl