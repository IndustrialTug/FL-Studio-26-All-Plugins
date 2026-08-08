#include "PluginScanner.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>

namespace fs = std::filesystem;

namespace FL {

void PluginScanner::addSearchPath(std::string path)
{
    auto it = std::find(m_searchPaths.begin(), m_searchPaths.end(), path);
    if (it == m_searchPaths.end())
        m_searchPaths.push_back(std::move(path));
}

void PluginScanner::removeSearchPath(const std::string& path)
{
    m_searchPaths.erase(
        std::remove(m_searchPaths.begin(), m_searchPaths.end(), path),
        m_searchPaths.end());
}

size_t PluginScanner::scan(FoundCallback cb)
{
    m_results.clear();
    for (const auto& root : m_searchPaths)
        walkDirectory(root, cb);
    return m_results.size();
}

void PluginScanner::walkDirectory(const std::string& dir, FoundCallback& cb)
{
    std::error_code ec;
    for (const auto& entry : fs::recursive_directory_iterator(dir,
             fs::directory_options::skip_permission_denied, ec))
    {
        if (ec) { ec.clear(); continue; }
        if (!entry.is_regular_file(ec)) { ec.clear(); continue; }

        PluginInfo info;
        if (probeFile(entry.path().string(), info))
        {
            m_results.push_back(info);
            if (cb) cb(info);
        }
    }
}

bool PluginScanner::probeFile(const std::string& filePath, PluginInfo& out) const
{
    const PluginFormat fmt = detectFormat(filePath);
    if (fmt == PluginFormat::Unknown)
        return false;

    out.filePath = filePath;
    out.format   = fmt;

    // Extract a display name from the filename without extension.
    fs::path p(filePath);
    out.name = p.stem().string();

    // In a real implementation we would load the shared library and query the
    // plugin descriptor.  Here we fill plausible defaults so the rest of the
    // system can work with the scanner output without requiring real plugins.
    out.vendor     = "Unknown Vendor";
    out.version    = "1.0.0";
    out.numInputs  = 2;
    out.numOutputs = 2;

    // Heuristic: names ending in common instrument keywords are likely synths.
    const std::string lower = [&]{
        std::string s = out.name;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }();
    const bool looksLikeInstrument =
        lower.find("synth") != std::string::npos ||
        lower.find("vsti")  != std::string::npos ||
        lower.find("sampl") != std::string::npos ||
        lower.find("piano") != std::string::npos ||
        lower.find("keys")  != std::string::npos;

    out.isInstrument = looksLikeInstrument;
    out.isSynth      = looksLikeInstrument;

    return true;
}

PluginFormat PluginScanner::detectFormat(const std::string& filePath) const noexcept
{
    const fs::path p(filePath);
    const std::string ext = p.extension().string();

    if (ext == ".dll") return PluginFormat::VST2;
    if (ext == ".vst3") return PluginFormat::VST3;
    if (ext == ".dylib" || ext == ".vst") return PluginFormat::AU;
    return PluginFormat::Unknown;
}

std::vector<PluginInfo> PluginScanner::filterByFormat(PluginFormat fmt) const
{
    std::vector<PluginInfo> out;
    for (const auto& p : m_results)
        if (p.format == fmt) out.push_back(p);
    return out;
}

std::vector<PluginInfo> PluginScanner::filterInstruments() const
{
    std::vector<PluginInfo> out;
    for (const auto& p : m_results)
        if (p.isInstrument) out.push_back(p);
    return out;
}

std::vector<PluginInfo> PluginScanner::filterEffects() const
{
    std::vector<PluginInfo> out;
    for (const auto& p : m_results)
        if (!p.isInstrument) out.push_back(p);
    return out;
}

} // namespace FL
