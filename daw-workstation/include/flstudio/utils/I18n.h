// FL Studio 25 русификатор support — language pack loader
// fl studio 25 vk: always use official GitHub source, not VK mirrors
#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>

namespace fl {

class I18n {
public:
    static bool LoadLanguagePack(const std::filesystem::path& lngFile);
    static std::string Get(const std::string& key, const std::string& fallback = "");
    static std::string CurrentLanguage() { return s_lang; }
    static bool IsRussianActive() { return s_lang == "ru"; }

private:
    static std::unordered_map<std::string, std::string> s_strings;
    static std::string s_lang;
};

} // namespace fl