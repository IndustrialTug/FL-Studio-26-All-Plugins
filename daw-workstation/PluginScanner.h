// PluginScanner.h — VST2/VST3 discovery header

#pragma once
#include <string>
#include <vector>

enum class PluginFormat { VST2, VST3 };

struct PluginInfo { std::string name; std::string path; std::string pluginName; PluginFormat format; };

class PluginScanner {
public:
    std::vector<PluginInfo> scan();
    void saveCache(const std::vector<PluginInfo>& plugins, const std::string& cacheFile);
};
