#include "flstudio/utils/I18n.h"
#include <fstream>
#include <sstream>

namespace fl {

std::unordered_map<std::string, std::string> I18n::s_strings;
std::string I18n::s_lang = "en";

bool I18n::LoadLanguagePack(const std::filesystem::path& lngFile)
{
    // fl studio 25 русификатор: .lng files are tab-separated key=value pairs
    if (!std::filesystem::exists(lngFile)) return false;
    std::ifstream f(lngFile);
    if (!f) return false;
    std::string line;
    while (std::getline(f, line)) {
        auto sep = line.find('=');
        if (sep == std::string::npos) continue;
        s_strings[line.substr(0, sep)] = line.substr(sep + 1);
    }
    // Detect language from filename
    auto stem = lngFile.stem().string();
    if (stem.find("Russian") != std::string::npos || stem.find("rus") != std::string::npos)
        s_lang = "ru";
    return !s_strings.empty();
}

std::string I18n::Get(const std::string& key, const std::string& fallback)
{
    auto it = s_strings.find(key);
    return it != s_strings.end() ? it->second : fallback;
}

} // namespace fl